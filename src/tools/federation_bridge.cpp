#include "tools/federation_bridge.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <sstream>
#include <utility>
#include <vector>

#include "tools/audit_log.h"
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

std::string buildRouteEndpointKey(const std::string &routeKey, const std::string &endpointId)
{
    return routeKey + "@" + toLower(endpointId);
}

std::vector<FederationBridgeConfig::EndpointConfig> activeEndpoints(const FederationBridgeConfig &config)
{
    std::vector<FederationBridgeConfig::EndpointConfig> endpoints;
    for (const auto &endpoint : config.endpoints)
    {
        if (endpoint.enabled)
        {
            endpoints.push_back(endpoint);
        }
    }
    if (endpoints.empty())
    {
        FederationBridgeConfig::EndpointConfig fallback;
        fallback.endpointId = "endpoint_default";
        fallback.outputFormatName = config.outputFormatName;
        fallback.enabled = true;
        endpoints.push_back(fallback);
    }
    return endpoints;
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
    if (!isValidToken(config.federateKeyId))
    {
        error = "federate_key_id is missing or invalid";
        return false;
    }
    if (config.federateKeyValidFromTimestampMs > config.federateKeyValidUntilTimestampMs)
    {
        error = "federate key validity window is invalid";
        return false;
    }
    if (config.requireFederateAttestation)
    {
        if (!isValidToken(config.federateAttestationTag))
        {
            error = "federate attestation tag is missing or invalid";
            return false;
        }
    }
    for (const auto &sourceId : config.allowedSourceIds)
    {
        if (!isValidToken(toLower(sourceId)))
        {
            error = "allowed source id is invalid";
            return false;
        }
    }
    std::vector<std::string> endpointIds;
    for (const auto &endpoint : config.endpoints)
    {
        if (!isValidToken(endpoint.endpointId))
        {
            error = "endpoint id is missing or invalid";
            return false;
        }
        if (!isSupportedIoEnvelopeFormat(endpoint.outputFormatName))
        {
            error = "unsupported endpoint output format: " + endpoint.outputFormatName;
            return false;
        }
        const std::string loweredEndpointId = toLower(endpoint.endpointId);
        if (std::find(endpointIds.begin(), endpointIds.end(), loweredEndpointId) != endpointIds.end())
        {
            error = "duplicate endpoint id";
            return false;
        }
        for (const auto &trustedKey : endpoint.acceptedFederateKeyIds)
        {
            if (!isValidToken(toLower(trustedKey)))
            {
                error = "endpoint accepted federate key id is invalid";
                return false;
            }
        }
        if (endpoint.requireFederateAttestation && !config.requireFederateAttestation)
        {
            error = "endpoint requires federate attestation";
            return false;
        }
        endpointIds.push_back(loweredEndpointId);
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

FederationFanoutResult FederationBridge::publishFanout(const ExternalIoEnvelope &envelope)
{
    FederationFanoutResult result;
    const std::string federateContext =
        "federate=" + toLower(config_.federateId) + " key_id=" + toLower(config_.federateKeyId);
    auto reject = [&](const std::string &reason, const std::string &detail = std::string()) -> FederationFanoutResult
    {
        result.error = reason;
        const std::string mergedDetail = detail.empty() ? federateContext : (federateContext + " " + detail);
        (void)logAuditEvent("federation_bridge_denied", reason, mergedDetail);
        return result;
    };

    if (envelope.metadata.schemaVersion.empty())
    {
        return reject("metadata.schema_version is required");
    }
    if (envelope.metadata.interfaceId.empty())
    {
        return reject("metadata.interface_id is required");
    }
    if (envelope.mode.activeMode.empty())
    {
        return reject("mode.active is required");
    }

    std::string configError;
    if (!validateConfig(config_, configError))
    {
        return reject(configError);
    }

    if (config_.requireDeterministic && !envelope.metadata.deterministic)
    {
        return reject("non-deterministic envelope rejected");
    }

    if (envelope.metadata.platformProfile.empty())
    {
        return reject("metadata.platform_profile is required");
    }
    const std::string sourceId = deriveSourceId(envelope);
    if (!isValidToken(sourceId))
    {
        return reject("source identifier is missing or invalid");
    }
    if (!allowedSourcesNormalized_.empty())
    {
        if (std::find(allowedSourcesNormalized_.begin(), allowedSourcesNormalized_.end(), sourceId) == allowedSourcesNormalized_.end())
        {
            return reject("source not allowed by route policy", "source=" + sourceId);
        }
    }
    if (!isValidToken(envelope.metadata.platformProfile))
    {
        return reject("metadata.platform_profile is invalid");
    }
    const std::string routeKey = buildRouteKey(config_.routeDomain, envelope.metadata.platformProfile, sourceId);
    const std::vector<FederationBridgeConfig::EndpointConfig> endpoints = activeEndpoints(config_);
    if (endpoints.empty())
    {
        return reject("no active endpoints");
    }

    if (willOverflowMul(nextLogicalTick_, config_.tickDurationMs))
    {
        return reject("timestamp overflow");
    }
    const std::uint64_t tickOffsetMs = nextLogicalTick_ * config_.tickDurationMs;
    if (willOverflowAdd(config_.startTimestampMs, tickOffsetMs))
    {
        return reject("timestamp overflow");
    }
    const std::uint64_t eventTimestampMs = config_.startTimestampMs + tickOffsetMs;
    if (eventTimestampMs < config_.federateKeyValidFromTimestampMs ||
        eventTimestampMs > config_.federateKeyValidUntilTimestampMs)
    {
        return reject("federate key material not valid for event timestamp", "route=" + routeKey);
    }
    const std::uint64_t sourceTimestampMs = envelope.frontView.timestampMs;
    if (config_.requireSourceTimestamp && sourceTimestampMs == 0U)
    {
        return reject("source timestamp is required", "route=" + routeKey);
    }
    if (sourceTimestampMs > 0U)
    {
        if (willOverflowAdd(eventTimestampMs, config_.maxFutureSkewMs))
        {
            return reject("time-authority overflow", "route=" + routeKey);
        }
        const std::uint64_t maxAuthorizedSourceTimestamp = eventTimestampMs + config_.maxFutureSkewMs;
        if (sourceTimestampMs > maxAuthorizedSourceTimestamp)
        {
            return reject("source timestamp ahead of time authority", "route=" + routeKey);
        }
        if (config_.requireMonotonicSourceTimestamp)
        {
            auto lastIt = lastSourceTimestampByKey_.find(routeKey);
            if (lastIt != lastSourceTimestampByKey_.end() && sourceTimestampMs < lastIt->second)
            {
                return reject("source timestamp regressed for route", "route=" + routeKey);
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
        return reject("latency budget exceeded", "route=" + routeKey);
    }

    std::vector<FederationEventFrame> frames;
    frames.reserve(endpoints.size());
    std::vector<std::string> routeEndpointKeys;
    routeEndpointKeys.reserve(endpoints.size());
    std::vector<std::uint64_t> routeSequences;
    routeSequences.reserve(endpoints.size());

    for (const auto &endpoint : endpoints)
    {
        const std::string normalizedEndpointId = toLower(endpoint.endpointId);
        if (!endpoint.acceptedFederateKeyIds.empty())
        {
            bool trustedKeyFound = false;
            const std::string normalizedFederateKeyId = toLower(config_.federateKeyId);
            for (const auto &trustedKey : endpoint.acceptedFederateKeyIds)
            {
                if (toLower(trustedKey) == normalizedFederateKeyId)
                {
                    trustedKeyFound = true;
                    break;
                }
            }
            if (!trustedKeyFound)
            {
                return reject("federate key id not trusted for endpoint", "endpoint=" + normalizedEndpointId);
            }
        }
        if (endpoint.requireFederateAttestation)
        {
            if (!config_.requireFederateAttestation || config_.federateAttestationTag.empty())
            {
                return reject("federate attestation required by endpoint policy", "endpoint=" + normalizedEndpointId);
            }
        }
        const std::string routeEndpointKey = buildRouteEndpointKey(routeKey, normalizedEndpointId);
        const std::uint64_t routeSequence = routeSequenceByKeyAndEndpoint_[routeEndpointKey];
        if (routeSequence == std::numeric_limits<std::uint64_t>::max())
        {
            return reject("route sequence overflow", "endpoint=" + normalizedEndpointId);
        }
        const IoEnvelopeSerializeResult serialized = serializeExternalIoEnvelope(endpoint.outputFormatName, envelope);
        if (!serialized.ok)
        {
            return reject(serialized.error, "endpoint=" + normalizedEndpointId);
        }

        FederationEventFrame frame;
        frame.schemaVersion = "1.0.0";
        frame.interfaceId = "airtrace.federation_event";
        frame.endpointId = normalizedEndpointId;
        frame.federateId = toLower(config_.federateId);
        frame.federateKeyId = toLower(config_.federateKeyId);
        frame.federateKeyEpoch = config_.federateKeyEpoch;
        frame.federateKeyValidUntilTimestampMs = config_.federateKeyValidUntilTimestampMs;
        frame.federateAttestationTag = config_.federateAttestationTag;
        frame.routeKey = routeKey;
        frame.routeSequence = routeSequence;
        frame.logicalTick = nextLogicalTick_;
        frame.eventTimestampMs = eventTimestampMs;
        frame.sourceTimestampMs = sourceTimestampMs;
        frame.sourceLatencyMs = latencyMs;
        frame.latencyBudgetMs = config_.maxLatencyBudgetMs;
        frame.sourceId = sourceId;
        frame.payloadFormat = ioEnvelopeFormatName(IoEnvelopeFormat::Json);
        IoEnvelopeFormat parsed = IoEnvelopeFormat::Json;
        if (parseIoEnvelopeFormat(endpoint.outputFormatName, parsed))
        {
            frame.payloadFormat = ioEnvelopeFormatName(parsed);
        }
        else
        {
            frame.payloadFormat = endpoint.outputFormatName;
        }
        frame.payload = serialized.payload;
        frame.seed = envelope.metadata.seed;
        frame.deterministic = envelope.metadata.deterministic;

        routeEndpointKeys.push_back(routeEndpointKey);
        routeSequences.push_back(routeSequence);
        frames.push_back(std::move(frame));
    }

    for (std::size_t idx = 0; idx < routeEndpointKeys.size(); ++idx)
    {
        routeSequenceByKeyAndEndpoint_[routeEndpointKeys[idx]] = routeSequences[idx] + 1U;
    }
    if (sourceTimestampMs > 0U)
    {
        lastSourceTimestampByKey_[routeKey] = sourceTimestampMs;
    }

    if (willOverflowAdd(nextLogicalTick_, config_.tickStep))
    {
        return reject("logical tick overflow");
    }
    nextLogicalTick_ += config_.tickStep;
    result.frames = std::move(frames);
    result.ok = true;
    (void)logAuditEvent(
        "federation_bridge_publish",
        "ok",
        federateContext + " route=" + routeKey + " endpoints=" + std::to_string(result.frames.size()));
    return result;
}

FederationBridgeResult FederationBridge::publish(const ExternalIoEnvelope &envelope)
{
    FederationBridgeResult result;
    const FederationFanoutResult fanout = publishFanout(envelope);
    if (!fanout.ok)
    {
        result.error = fanout.error;
        return result;
    }
    if (fanout.frames.empty())
    {
        result.error = "fanout produced no frames";
        return result;
    }
    result.ok = true;
    result.frame = fanout.frames.front();
    return result;
}

std::string serializeFederationEventFrameJson(const FederationEventFrame &frame)
{
    std::ostringstream out;
    out << "{";
    out << "\"schema_version\":\"" << jsonEscape(frame.schemaVersion) << "\",";
    out << "\"interface_id\":\"" << jsonEscape(frame.interfaceId) << "\",";
    out << "\"endpoint_id\":\"" << jsonEscape(frame.endpointId) << "\",";
    out << "\"federate_id\":\"" << jsonEscape(frame.federateId) << "\",";
    out << "\"federate_key_id\":\"" << jsonEscape(frame.federateKeyId) << "\",";
    out << "\"federate_key_epoch\":" << frame.federateKeyEpoch << ",";
    out << "\"federate_key_valid_until_timestamp_ms\":" << frame.federateKeyValidUntilTimestampMs << ",";
    out << "\"federate_attestation_tag\":\"" << jsonEscape(frame.federateAttestationTag) << "\",";
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
