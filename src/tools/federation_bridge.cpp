#include "tools/federation_bridge.h"

#include <limits>
#include <sstream>
#include <utility>

#include "tools/io_packager.h"

namespace tools
{
namespace
{
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
    result.frame.logicalTick = nextLogicalTick_;
    result.frame.eventTimestampMs = eventTimestampMs;
    result.frame.sourceTimestampMs = sourceTimestampMs;
    result.frame.sourceLatencyMs = latencyMs;
    result.frame.latencyBudgetMs = config_.maxLatencyBudgetMs;
    result.frame.sourceId = envelope.frontView.sourceId.empty() ? envelope.mode.activeMode : envelope.frontView.sourceId;
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
