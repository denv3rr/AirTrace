#include "tools/federation_bridge.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <utility>

#include "tools/io_packager.h"

namespace tools
{
namespace
{
std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool isValidToken(const std::string &value)
{
    if (value.empty())
    {
        return false;
    }
    for (char ch : value)
    {
        if (!(std::isalnum(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-' || ch == '.'))
        {
            return false;
        }
    }
    return true;
}

std::string deriveSourceId(const ExternalIoEnvelope &envelope)
{
    if (!envelope.frontView.sourceId.empty())
    {
        return toLower(envelope.frontView.sourceId);
    }
    return toLower(envelope.mode.activeMode);
}

std::string buildRouteKey(
    const std::string &routeDomain,
    const std::string &platformProfile,
    const std::string &sourceId)
{
    return toLower(routeDomain) + "/" + toLower(platformProfile) + "/" + toLower(sourceId);
}

bool validateConfig(const FederationBridgeConfig &config, std::string &error)
{
    if (config.tickStep == 0)
    {
        error = "tick_step must be > 0";
        return false;
    }
    if (config.tickDurationMs == 0)
    {
        error = "tick_duration_ms must be > 0";
        return false;
    }
    if (config.maxLatencyBudgetMs < 0.0)
    {
        error = "max_latency_budget_ms must be >= 0";
        return false;
    }
    if (!isSupportedIoEnvelopeFormat(config.outputFormatName))
    {
        error = "unsupported output format: " + config.outputFormatName;
        return false;
    }
    if (!isValidToken(config.federateId))
    {
        error = "federate_id is missing or invalid";
        return false;
    }
    if (!isValidToken(config.routeDomain))
    {
        error = "route_domain is missing or invalid";
        return false;
    }
    for (const auto &sourceId : config.allowedSourceIds)
    {
        if (!isValidToken(toLower(sourceId)))
        {
            error = "allowed source id is invalid";
            return false;
        }
    }
    return true;
}

bool willOverflowMul(std::uint64_t a, std::uint64_t b)
{
    if (a == 0 || b == 0)
    {
        return false;
    }
    return a > (std::numeric_limits<std::uint64_t>::max() / b);
}

bool willOverflowAdd(std::uint64_t a, std::uint64_t b)
{
    return a > (std::numeric_limits<std::uint64_t>::max() - b);
}

std::string jsonEscape(const std::string &value)
{
    std::string escaped;
    escaped.reserve(value.size() + 8);
    for (char ch : value)
    {
        switch (ch)
        {
        case '\\':
            escaped += "\\\\";
            break;
        case '"':
            escaped += "\\\"";
            break;
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        case '\t':
            escaped += "\\t";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}
} // namespace

FederationBridge::FederationBridge(FederationBridgeConfig config)
    : config_(std::move(config)),
      nextLogicalTick_(config_.startLogicalTick)
{
    allowedSourcesNormalized_.reserve(config_.allowedSourceIds.size());
    for (const auto &sourceId : config_.allowedSourceIds)
    {
        allowedSourcesNormalized_.push_back(toLower(sourceId));
    }
}

FederationBridgeResult FederationBridge::publish(const ExternalIoEnvelope &envelope)
{
    FederationBridgeResult result;

    if (envelope.metadata.schemaVersion.empty())
    {
        result.error = "metadata.schema_version is required";
        return result;
    }
    if (envelope.metadata.interfaceId.empty())
    {
        result.error = "metadata.interface_id is required";
        return result;
    }
    if (envelope.mode.activeMode.empty())
    {
        result.error = "mode.active is required";
        return result;
    }

    std::string configError;
    if (!validateConfig(config_, configError))
    {
        result.error = configError;
        return result;
    }

    if (config_.requireDeterministic && !envelope.metadata.deterministic)
    {
        result.error = "non-deterministic envelope rejected";
        return result;
    }

    if (envelope.metadata.platformProfile.empty())
    {
        result.error = "metadata.platform_profile is required";
        return result;
    }
    const std::string sourceId = deriveSourceId(envelope);
    if (!isValidToken(sourceId))
    {
        result.error = "source identifier is missing or invalid";
        return result;
    }
    if (!allowedSourcesNormalized_.empty())
    {
        if (std::find(allowedSourcesNormalized_.begin(), allowedSourcesNormalized_.end(), sourceId) == allowedSourcesNormalized_.end())
        {
            result.error = "source not allowed by route policy";
            return result;
        }
    }
    if (!isValidToken(envelope.metadata.platformProfile))
    {
        result.error = "metadata.platform_profile is invalid";
        return result;
    }
    const std::string routeKey = buildRouteKey(config_.routeDomain, envelope.metadata.platformProfile, sourceId);

    if (willOverflowMul(nextLogicalTick_, config_.tickDurationMs))
    {
        result.error = "timestamp overflow";
        return result;
    }
    const std::uint64_t tickOffsetMs = nextLogicalTick_ * config_.tickDurationMs;
    if (willOverflowAdd(config_.startTimestampMs, tickOffsetMs))
    {
        result.error = "timestamp overflow";
        return result;
    }
    const std::uint64_t eventTimestampMs = config_.startTimestampMs + tickOffsetMs;
    const std::uint64_t sourceTimestampMs = envelope.frontView.timestampMs;
    if (config_.requireSourceTimestamp && sourceTimestampMs == 0U)
    {
        result.error = "source timestamp is required";
        return result;
    }
    if (sourceTimestampMs > 0U)
    {
        if (willOverflowAdd(eventTimestampMs, config_.maxFutureSkewMs))
        {
            result.error = "time-authority overflow";
            return result;
        }
        const std::uint64_t maxAuthorizedSourceTimestamp = eventTimestampMs + config_.maxFutureSkewMs;
        if (sourceTimestampMs > maxAuthorizedSourceTimestamp)
        {
            result.error = "source timestamp ahead of time authority";
            return result;
        }
        if (config_.requireMonotonicSourceTimestamp)
        {
            auto lastIt = lastSourceTimestampByKey_.find(routeKey);
            if (lastIt != lastSourceTimestampByKey_.end() && sourceTimestampMs < lastIt->second)
            {
                result.error = "source timestamp regressed for route";
                return result;
            }
        }
    }

    double latencyMs = 0.0;
    if (sourceTimestampMs > 0 && eventTimestampMs >= sourceTimestampMs)
    {
        latencyMs = static_cast<double>(eventTimestampMs - sourceTimestampMs);
    }
    if (latencyMs > config_.maxLatencyBudgetMs)
    {
        result.error = "latency budget exceeded";
        return result;
    }

    const IoEnvelopeSerializeResult serialized = serializeExternalIoEnvelope(config_.outputFormatName, envelope);
    if (!serialized.ok)
    {
        result.error = serialized.error;
        return result;
    }

    result.frame.schemaVersion = "1.0.0";
    result.frame.interfaceId = "airtrace.federation_event";
    result.frame.federateId = toLower(config_.federateId);
    result.frame.routeKey = routeKey;
    result.frame.routeSequence = routeSequenceByKey_[routeKey];
    result.frame.logicalTick = nextLogicalTick_;
    result.frame.eventTimestampMs = eventTimestampMs;
    result.frame.sourceTimestampMs = sourceTimestampMs;
    result.frame.sourceLatencyMs = latencyMs;
    result.frame.latencyBudgetMs = config_.maxLatencyBudgetMs;
    result.frame.sourceId = sourceId;
    result.frame.payloadFormat = ioEnvelopeFormatName(IoEnvelopeFormat::Json);
    if (!config_.outputFormatName.empty())
    {
        IoEnvelopeFormat parsed = IoEnvelopeFormat::Json;
        if (parseIoEnvelopeFormat(config_.outputFormatName, parsed))
        {
            result.frame.payloadFormat = ioEnvelopeFormatName(parsed);
        }
        else
        {
            result.frame.payloadFormat = config_.outputFormatName;
        }
    }
    result.frame.payload = serialized.payload;
    result.frame.seed = envelope.metadata.seed;
    result.frame.deterministic = envelope.metadata.deterministic;
    lastSourceTimestampByKey_[routeKey] = sourceTimestampMs;
    if (routeSequenceByKey_[routeKey] == std::numeric_limits<std::uint64_t>::max())
    {
        result.error = "route sequence overflow";
        return result;
    }
    routeSequenceByKey_[routeKey] += 1U;

    if (willOverflowAdd(nextLogicalTick_, config_.tickStep))
    {
        result.error = "logical tick overflow";
        return result;
    }
    nextLogicalTick_ += config_.tickStep;
    result.ok = true;
    return result;
}

std::string serializeFederationEventFrameJson(const FederationEventFrame &frame)
{
    std::ostringstream out;
    out << "{";
    out << "\"schema_version\":\"" << jsonEscape(frame.schemaVersion) << "\",";
    out << "\"interface_id\":\"" << jsonEscape(frame.interfaceId) << "\",";
    out << "\"federate_id\":\"" << jsonEscape(frame.federateId) << "\",";
    out << "\"route_key\":\"" << jsonEscape(frame.routeKey) << "\",";
    out << "\"route_sequence\":" << frame.routeSequence << ",";
    out << "\"logical_tick\":" << frame.logicalTick << ",";
    out << "\"event_timestamp_ms\":" << frame.eventTimestampMs << ",";
    out << "\"source_timestamp_ms\":" << frame.sourceTimestampMs << ",";
    out << "\"source_latency_ms\":" << frame.sourceLatencyMs << ",";
    out << "\"latency_budget_ms\":" << frame.latencyBudgetMs << ",";
    out << "\"source_id\":\"" << jsonEscape(frame.sourceId) << "\",";
    out << "\"payload_format\":\"" << jsonEscape(frame.payloadFormat) << "\",";
    out << "\"seed\":" << frame.seed << ",";
    out << "\"deterministic\":" << (frame.deterministic ? "true" : "false") << ",";
    out << "\"payload\":\"" << jsonEscape(frame.payload) << "\"";
    out << "}";
    return out.str();
}
} // namespace tools
