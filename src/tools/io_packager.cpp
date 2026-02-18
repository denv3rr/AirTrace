#include "tools/io_packager.h"

#include <cmath>
#include <cctype>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <map>
#include <sstream>
#include <unordered_set>
#include <vector>

namespace tools
{
namespace
{
std::string toLower(std::string value)
{
    for (char &ch : value)
    {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

std::string trim(std::string value)
{
    std::size_t start = 0;
    while (start < value.size() && std::isspace(static_cast<unsigned char>(value[start])))
    {
        ++start;
    }
    std::size_t end = value.size();
    while (end > start && std::isspace(static_cast<unsigned char>(value[end - 1])))
    {
        --end;
    }
    return value.substr(start, end - start);
}

struct BuiltinCodec
{
    IoEnvelopeFormat format = IoEnvelopeFormat::Json;
    const char *canonicalName = "";
    std::vector<std::string> aliases{};
};

const std::vector<BuiltinCodec> &builtinCodecs()
{
    static const std::vector<BuiltinCodec> kCodecs = {
        {IoEnvelopeFormat::Json, "ie_json_v1", {"json"}},
        {IoEnvelopeFormat::KeyValue, "ie_kv_v1", {"kv", "keyvalue"}}};
    return kCodecs;
}

const BuiltinCodec *findCodecByFormat(IoEnvelopeFormat format)
{
    const auto &codecs = builtinCodecs();
    for (const auto &codec : codecs)
    {
        if (codec.format == format)
        {
            return &codec;
        }
    }
    return nullptr;
}

const BuiltinCodec *findCodecByName(const std::string &text)
{
    const std::string normalized = toLower(trim(text));
    const auto &codecs = builtinCodecs();
    for (const auto &codec : codecs)
    {
        if (normalized == codec.canonicalName)
        {
            return &codec;
        }
        for (const auto &alias : codec.aliases)
        {
            if (normalized == alias)
            {
                return &codec;
            }
        }
    }
    return nullptr;
}

bool parseBoolStrict(const std::string &value, bool &out)
{
    const std::string lowered = toLower(value);
    if (lowered == "true" || lowered == "1")
    {
        out = true;
        return true;
    }
    if (lowered == "false" || lowered == "0")
    {
        out = false;
        return true;
    }
    return false;
}

bool parseUnsignedStrict(const std::string &value, unsigned int &out)
{
    if (value.empty())
    {
        return false;
    }
    for (char ch : value)
    {
        if (!std::isdigit(static_cast<unsigned char>(ch)))
        {
            return false;
        }
    }
    char *endPtr = nullptr;
    const unsigned long parsed = std::strtoul(value.c_str(), &endPtr, 10);
    if (endPtr == nullptr || *endPtr != '\0' ||
        parsed > static_cast<unsigned long>(std::numeric_limits<unsigned int>::max()))
    {
        return false;
    }
    out = static_cast<unsigned int>(parsed);
    return true;
}

bool parseUInt64Strict(const std::string &value, std::uint64_t &out)
{
    if (value.empty())
    {
        return false;
    }
    for (char ch : value)
    {
        if (!std::isdigit(static_cast<unsigned char>(ch)))
        {
            return false;
        }
    }
    char *endPtr = nullptr;
    const unsigned long long parsed = std::strtoull(value.c_str(), &endPtr, 10);
    if (endPtr == nullptr || *endPtr != '\0')
    {
        return false;
    }
    out = static_cast<std::uint64_t>(parsed);
    return true;
}

bool parseIntStrict(const std::string &value, int &out)
{
    if (value.empty())
    {
        return false;
    }
    char *endPtr = nullptr;
    const long parsed = std::strtol(value.c_str(), &endPtr, 10);
    if (endPtr == nullptr || *endPtr != '\0' ||
        parsed < static_cast<long>(std::numeric_limits<int>::min()) ||
        parsed > static_cast<long>(std::numeric_limits<int>::max()))
    {
        return false;
    }
    out = static_cast<int>(parsed);
    return true;
}

bool parseDoubleStrict(const std::string &value, double &out)
{
    if (value.empty())
    {
        return false;
    }
    char *endPtr = nullptr;
    const double parsed = std::strtod(value.c_str(), &endPtr);
    if (endPtr == nullptr || *endPtr != '\0')
    {
        return false;
    }
    if (!std::isfinite(parsed))
    {
        return false;
    }
    out = parsed;
    return true;
}

std::string formatDoubleExact(double value)
{
    std::ostringstream stream;
    stream << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    return stream.str();
}

std::vector<std::string> split(const std::string &value, char delimiter)
{
    std::vector<std::string> parts;
    std::string current;
    for (char ch : value)
    {
        if (ch == delimiter)
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(ch);
        }
    }
    parts.push_back(current);
    return parts;
}

std::string kvEscape(const std::string &value)
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
        case '\n':
            escaped += "\\n";
            break;
        case '\r':
            escaped += "\\r";
            break;
        default:
            escaped.push_back(ch);
            break;
        }
    }
    return escaped;
}

bool kvUnescape(const std::string &value, std::string &out)
{
    out.clear();
    out.reserve(value.size());
    bool escape = false;
    for (char ch : value)
    {
        if (!escape)
        {
            if (ch == '\\')
            {
                escape = true;
            }
            else
            {
                out.push_back(ch);
            }
            continue;
        }

        if (ch == 'n')
        {
            out.push_back('\n');
        }
        else if (ch == 'r')
        {
            out.push_back('\r');
        }
        else if (ch == '\\')
        {
            out.push_back('\\');
        }
        else
        {
            return false;
        }
        escape = false;
    }
    return !escape;
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

bool jsonParseString(const std::string &text, std::size_t &pos, std::string &value)
{
    if (pos >= text.size() || text[pos] != '"')
    {
        return false;
    }
    ++pos;
    value.clear();
    while (pos < text.size())
    {
        const char ch = text[pos++];
        if (ch == '"')
        {
            return true;
        }
        if (ch != '\\')
        {
            value.push_back(ch);
            continue;
        }
        if (pos >= text.size())
        {
            return false;
        }
        const char esc = text[pos++];
        switch (esc)
        {
        case '"':
            value.push_back('"');
            break;
        case '\\':
            value.push_back('\\');
            break;
        case 'n':
            value.push_back('\n');
            break;
        case 'r':
            value.push_back('\r');
            break;
        case 't':
            value.push_back('\t');
            break;
        default:
            return false;
        }
    }
    return false;
}

bool jsonMatchLiteral(const std::string &text, std::size_t &pos, const char *literal)
{
    std::size_t idx = 0;
    while (literal[idx] != '\0')
    {
        if (pos >= text.size() || text[pos] != literal[idx])
        {
            return false;
        }
        ++pos;
        ++idx;
    }
    return true;
}

bool jsonParseNumber(const std::string &text, std::size_t &pos, std::string &value)
{
    const std::size_t start = pos;
    if (pos < text.size() && text[pos] == '-')
    {
        ++pos;
    }
    if (pos >= text.size())
    {
        return false;
    }
    if (text[pos] == '0')
    {
        ++pos;
    }
    else if (std::isdigit(static_cast<unsigned char>(text[pos])) != 0)
    {
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])) != 0)
        {
            ++pos;
        }
    }
    else
    {
        return false;
    }
    if (pos < text.size() && text[pos] == '.')
    {
        ++pos;
        if (pos >= text.size() || std::isdigit(static_cast<unsigned char>(text[pos])) == 0)
        {
            return false;
        }
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])) != 0)
        {
            ++pos;
        }
    }
    if (pos < text.size() && (text[pos] == 'e' || text[pos] == 'E'))
    {
        ++pos;
        if (pos < text.size() && (text[pos] == '+' || text[pos] == '-'))
        {
            ++pos;
        }
        if (pos >= text.size() || std::isdigit(static_cast<unsigned char>(text[pos])) == 0)
        {
            return false;
        }
        while (pos < text.size() && std::isdigit(static_cast<unsigned char>(text[pos])) != 0)
        {
            ++pos;
        }
    }
    value = text.substr(start, pos - start);
    return true;
}

bool jsonParseValue(const std::string &text, std::size_t &pos, std::string &value, std::string &error)
{
    if (pos >= text.size())
    {
        error = "json unexpected end";
        return false;
    }
    const char ch = text[pos];
    if (ch == '"')
    {
        if (!jsonParseString(text, pos, value))
        {
            error = "json invalid string value";
            return false;
        }
        return true;
    }
    if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)) != 0)
    {
        if (!jsonParseNumber(text, pos, value))
        {
            error = "json invalid numeric value";
            return false;
        }
        return true;
    }
    if (ch == 't')
    {
        if (!jsonMatchLiteral(text, pos, "true"))
        {
            error = "json invalid literal";
            return false;
        }
        value = "true";
        return true;
    }
    if (ch == 'f')
    {
        if (!jsonMatchLiteral(text, pos, "false"))
        {
            error = "json invalid literal";
            return false;
        }
        value = "false";
        return true;
    }
    if (ch == 'n')
    {
        error = "json null values are not supported";
        return false;
    }
    if (ch == '{' || ch == '[')
    {
        error = "json nested values are not supported";
        return false;
    }
    error = "json invalid value";
    return false;
}

void jsonSkipWhitespace(const std::string &text, std::size_t &pos)
{
    while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])) != 0)
    {
        ++pos;
    }
}

bool parseFlatJson(const std::string &payload, std::map<std::string, std::string> &out, std::string &error)
{
    out.clear();
    std::size_t pos = 0;
    jsonSkipWhitespace(payload, pos);
    if (pos >= payload.size() || payload[pos] != '{')
    {
        error = "json must start with '{'";
        return false;
    }
    ++pos;
    jsonSkipWhitespace(payload, pos);
    if (pos < payload.size() && payload[pos] == '}')
    {
        ++pos;
        return true;
    }

    while (pos < payload.size())
    {
        jsonSkipWhitespace(payload, pos);
        std::string key;
        if (!jsonParseString(payload, pos, key))
        {
            error = "json invalid key";
            return false;
        }
        jsonSkipWhitespace(payload, pos);
        if (pos >= payload.size() || payload[pos] != ':')
        {
            error = "json missing ':'";
            return false;
        }
        ++pos;
        jsonSkipWhitespace(payload, pos);

        std::string value;
        if (!jsonParseValue(payload, pos, value, error))
        {
            return false;
        }
        if (out.find(key) != out.end())
        {
            error = "json duplicate key: " + key;
            return false;
        }
        out[key] = value;

        jsonSkipWhitespace(payload, pos);
        if (pos >= payload.size())
        {
            error = "json unexpected end";
            return false;
        }
        if (payload[pos] == '}')
        {
            ++pos;
            jsonSkipWhitespace(payload, pos);
            if (pos != payload.size())
            {
                error = "json trailing content";
                return false;
            }
            return true;
        }
        if (payload[pos] != ',')
        {
            error = "json missing ','";
            return false;
        }
        ++pos;
    }

    error = "json unexpected end";
    return false;
}

void flattenEnvelope(const ExternalIoEnvelope &envelope, std::map<std::string, std::string> &flat)
{
    flat.clear();
    flat["schema_version"] = envelope.metadata.schemaVersion;
    flat["interface_id"] = envelope.metadata.interfaceId;
    flat["metadata.platform_profile"] = envelope.metadata.platformProfile;
    flat["metadata.adapter_id"] = envelope.metadata.adapterId;
    flat["metadata.adapter_version"] = envelope.metadata.adapterVersion;
    flat["metadata.ui_surface"] = envelope.metadata.uiSurface;
    flat["metadata.seed"] = std::to_string(envelope.metadata.seed);
    flat["metadata.deterministic"] = envelope.metadata.deterministic ? "true" : "false";

    flat["mode.active"] = envelope.mode.activeMode;
    flat["mode.confidence"] = formatDoubleExact(envelope.mode.confidence);
    flat["mode.decision_reason"] = envelope.mode.decisionReason;
    flat["mode.denial_reason"] = envelope.mode.denialReason;
    flat["mode.ladder_status"] = envelope.mode.ladderStatus;
    {
        std::ostringstream contributors;
        for (std::size_t idx = 0; idx < envelope.mode.contributors.size(); ++idx)
        {
            contributors << envelope.mode.contributors[idx];
            if (idx + 1 < envelope.mode.contributors.size())
            {
                contributors << ",";
            }
        }
        flat["mode.contributors"] = contributors.str();
    }

    flat["sensor.count"] = std::to_string(envelope.sensors.size());
    for (std::size_t idx = 0; idx < envelope.sensors.size(); ++idx)
    {
        const auto &sensor = envelope.sensors[idx];
        const std::string prefix = "sensor." + std::to_string(idx) + ".";
        flat[prefix + "id"] = sensor.sensorId;
        flat[prefix + "available"] = sensor.available ? "true" : "false";
        flat[prefix + "healthy"] = sensor.healthy ? "true" : "false";
        flat[prefix + "has_measurement"] = sensor.hasMeasurement ? "true" : "false";
        flat[prefix + "freshness_seconds"] = formatDoubleExact(sensor.freshnessSeconds);
        flat[prefix + "confidence"] = formatDoubleExact(sensor.confidence);
        flat[prefix + "last_error"] = sensor.lastError;
    }

    flat["front_view.active_mode"] = envelope.frontView.activeMode;
    flat["front_view.view_state"] = envelope.frontView.viewState;
    flat["front_view.frame_id"] = envelope.frontView.frameId;
    flat["front_view.source_id"] = envelope.frontView.sourceId;
    flat["front_view.sensor_type"] = envelope.frontView.sensorType;
    flat["front_view.sequence"] = std::to_string(envelope.frontView.sequence);
    flat["front_view.timestamp_ms"] = std::to_string(envelope.frontView.timestampMs);
    flat["front_view.frame_age_ms"] = formatDoubleExact(envelope.frontView.frameAgeMs);
    flat["front_view.acquisition_latency_ms"] = formatDoubleExact(envelope.frontView.acquisitionLatencyMs);
    flat["front_view.processing_latency_ms"] = formatDoubleExact(envelope.frontView.processingLatencyMs);
    flat["front_view.render_latency_ms"] = formatDoubleExact(envelope.frontView.renderLatencyMs);
    flat["front_view.latency_ms"] = formatDoubleExact(envelope.frontView.latencyMs);
    flat["front_view.dropped_frames"] = std::to_string(envelope.frontView.droppedFrames);
    flat["front_view.drop_reason"] = envelope.frontView.dropReason;
    flat["front_view.spoof_active"] = envelope.frontView.spoofActive ? "true" : "false";
    flat["front_view.confidence"] = formatDoubleExact(envelope.frontView.confidence);
    flat["front_view.provenance"] = envelope.frontView.provenance;
    flat["front_view.auth_status"] = envelope.frontView.authStatus;
    flat["front_view.stream_id"] = envelope.frontView.streamId;
    flat["front_view.stream_index"] = std::to_string(envelope.frontView.streamIndex);
    flat["front_view.stream_count"] = std::to_string(envelope.frontView.streamCount);
    flat["front_view.max_concurrent_views"] = std::to_string(envelope.frontView.maxConcurrentViews);
    flat["front_view.stabilization_mode"] = envelope.frontView.stabilizationMode;
    flat["front_view.stabilization_active"] = envelope.frontView.stabilizationActive ? "true" : "false";
    flat["front_view.stabilization_error_deg"] = formatDoubleExact(envelope.frontView.stabilizationErrorDeg);
    flat["front_view.gimbal_yaw_deg"] = formatDoubleExact(envelope.frontView.gimbalYawDeg);
    flat["front_view.gimbal_pitch_deg"] = formatDoubleExact(envelope.frontView.gimbalPitchDeg);
    flat["front_view.gimbal_yaw_rate_deg_s"] = formatDoubleExact(envelope.frontView.gimbalYawRateDegPerSec);
    flat["front_view.gimbal_pitch_rate_deg_s"] = formatDoubleExact(envelope.frontView.gimbalPitchRateDegPerSec);

    flat["front_view_stream.count"] = std::to_string(envelope.frontViewStreams.size());
    for (std::size_t idx = 0; idx < envelope.frontViewStreams.size(); ++idx)
    {
        const auto &stream = envelope.frontViewStreams[idx];
        const std::string prefix = "front_view_stream." + std::to_string(idx) + ".";
        flat[prefix + "stream_id"] = stream.streamId;
        flat[prefix + "active_mode"] = stream.activeMode;
        flat[prefix + "frame_id"] = stream.frameId;
        flat[prefix + "sensor_type"] = stream.sensorType;
        flat[prefix + "sequence"] = std::to_string(stream.sequence);
        flat[prefix + "timestamp_ms"] = std::to_string(stream.timestampMs);
        flat[prefix + "frame_age_ms"] = formatDoubleExact(stream.frameAgeMs);
        flat[prefix + "latency_ms"] = formatDoubleExact(stream.latencyMs);
        flat[prefix + "confidence"] = formatDoubleExact(stream.confidence);
        flat[prefix + "stabilization_mode"] = stream.stabilizationMode;
        flat[prefix + "stabilization_active"] = stream.stabilizationActive ? "true" : "false";
    }

    flat["status.disqualified_sources"] = envelope.disqualifiedSources;
    flat["status.lockout_status"] = envelope.lockoutStatus;
    flat["status.auth_status"] = envelope.authStatus;
    flat["status.provenance_status"] = envelope.provenanceStatus;
    flat["status.logging_status"] = envelope.loggingStatus;
    flat["status.adapter_status"] = envelope.adapterStatus;
    flat["status.adapter_reason"] = envelope.adapterReason;
    flat["status.adapter_fields"] = envelope.adapterFields;
}

bool inflateEnvelope(const std::map<std::string, std::string> &flat, ExternalIoEnvelope &envelope, std::string &error)
{
    auto get = [&](const std::string &key, std::string &out) -> bool
    {
        const auto it = flat.find(key);
        if (it == flat.end())
        {
            error = "missing key: " + key;
            return false;
        }
        out = it->second;
        return true;
    };
    auto optional = [&](const std::string &key, std::string &out) -> void
    {
        const auto it = flat.find(key);
        if (it != flat.end())
        {
            out = it->second;
        }
    };

    std::string text;
    if (!get("schema_version", envelope.metadata.schemaVersion) ||
        !get("interface_id", envelope.metadata.interfaceId) ||
        !get("metadata.platform_profile", envelope.metadata.platformProfile) ||
        !get("metadata.adapter_id", envelope.metadata.adapterId) ||
        !get("metadata.adapter_version", envelope.metadata.adapterVersion) ||
        !get("metadata.ui_surface", envelope.metadata.uiSurface))
    {
        return false;
    }
    if (!get("metadata.seed", text) || !parseUnsignedStrict(text, envelope.metadata.seed))
    {
        error = "invalid metadata.seed";
        return false;
    }
    if (!get("metadata.deterministic", text) || !parseBoolStrict(text, envelope.metadata.deterministic))
    {
        error = "invalid metadata.deterministic";
        return false;
    }

    if (!get("mode.active", envelope.mode.activeMode))
    {
        return false;
    }
    if (!get("mode.confidence", text) || !parseDoubleStrict(text, envelope.mode.confidence))
    {
        error = "invalid mode.confidence";
        return false;
    }
    optional("mode.decision_reason", envelope.mode.decisionReason);
    optional("mode.denial_reason", envelope.mode.denialReason);
    optional("mode.ladder_status", envelope.mode.ladderStatus);
    optional("mode.contributors", text);
    if (!text.empty())
    {
        envelope.mode.contributors = split(text, ',');
    }

    unsigned int sensorCount = 0;
    if (!get("sensor.count", text) || !parseUnsignedStrict(text, sensorCount))
    {
        error = "invalid sensor.count";
        return false;
    }
    envelope.sensors.clear();
    std::unordered_set<std::string> sensorIds;
    for (unsigned int idx = 0; idx < sensorCount; ++idx)
    {
        const std::string prefix = "sensor." + std::to_string(idx) + ".";
        ExternalIoSensorRecord sensor;
        if (!get(prefix + "id", sensor.sensorId))
        {
            return false;
        }
        if (!sensorIds.insert(sensor.sensorId).second)
        {
            error = "duplicate sensor id: " + sensor.sensorId;
            return false;
        }
        if (!get(prefix + "available", text) || !parseBoolStrict(text, sensor.available) ||
            !get(prefix + "healthy", text) || !parseBoolStrict(text, sensor.healthy) ||
            !get(prefix + "has_measurement", text) || !parseBoolStrict(text, sensor.hasMeasurement) ||
            !get(prefix + "freshness_seconds", text) || !parseDoubleStrict(text, sensor.freshnessSeconds) ||
            !get(prefix + "confidence", text) || !parseDoubleStrict(text, sensor.confidence))
        {
            error = "invalid sensor value at index " + std::to_string(idx);
            return false;
        }
        optional(prefix + "last_error", sensor.lastError);
        envelope.sensors.push_back(sensor);
    }

    optional("front_view.active_mode", envelope.frontView.activeMode);
    optional("front_view.view_state", envelope.frontView.viewState);
    optional("front_view.frame_id", envelope.frontView.frameId);
    optional("front_view.source_id", envelope.frontView.sourceId);
    optional("front_view.sensor_type", envelope.frontView.sensorType);
    if (!get("front_view.sequence", text) || !parseUnsignedStrict(text, envelope.frontView.sequence) ||
        !get("front_view.timestamp_ms", text) || !parseUInt64Strict(text, envelope.frontView.timestampMs) ||
        !get("front_view.frame_age_ms", text) || !parseDoubleStrict(text, envelope.frontView.frameAgeMs) ||
        !get("front_view.acquisition_latency_ms", text) || !parseDoubleStrict(text, envelope.frontView.acquisitionLatencyMs) ||
        !get("front_view.processing_latency_ms", text) || !parseDoubleStrict(text, envelope.frontView.processingLatencyMs) ||
        !get("front_view.render_latency_ms", text) || !parseDoubleStrict(text, envelope.frontView.renderLatencyMs) ||
        !get("front_view.latency_ms", text) || !parseDoubleStrict(text, envelope.frontView.latencyMs) ||
        !get("front_view.dropped_frames", text) || !parseIntStrict(text, envelope.frontView.droppedFrames) ||
        !get("front_view.spoof_active", text) || !parseBoolStrict(text, envelope.frontView.spoofActive) ||
        !get("front_view.confidence", text) || !parseDoubleStrict(text, envelope.frontView.confidence) ||
        !get("front_view.stream_index", text) || !parseUnsignedStrict(text, envelope.frontView.streamIndex) ||
        !get("front_view.stream_count", text) || !parseUnsignedStrict(text, envelope.frontView.streamCount) ||
        !get("front_view.max_concurrent_views", text) || !parseUnsignedStrict(text, envelope.frontView.maxConcurrentViews) ||
        !get("front_view.stabilization_active", text) || !parseBoolStrict(text, envelope.frontView.stabilizationActive) ||
        !get("front_view.stabilization_error_deg", text) || !parseDoubleStrict(text, envelope.frontView.stabilizationErrorDeg) ||
        !get("front_view.gimbal_yaw_deg", text) || !parseDoubleStrict(text, envelope.frontView.gimbalYawDeg) ||
        !get("front_view.gimbal_pitch_deg", text) || !parseDoubleStrict(text, envelope.frontView.gimbalPitchDeg) ||
        !get("front_view.gimbal_yaw_rate_deg_s", text) || !parseDoubleStrict(text, envelope.frontView.gimbalYawRateDegPerSec) ||
        !get("front_view.gimbal_pitch_rate_deg_s", text) || !parseDoubleStrict(text, envelope.frontView.gimbalPitchRateDegPerSec))
    {
        error = "invalid front_view values";
        return false;
    }
    optional("front_view.drop_reason", envelope.frontView.dropReason);
    optional("front_view.provenance", envelope.frontView.provenance);
    optional("front_view.auth_status", envelope.frontView.authStatus);
    optional("front_view.stream_id", envelope.frontView.streamId);
    optional("front_view.stabilization_mode", envelope.frontView.stabilizationMode);

    unsigned int streamCount = 0;
    if (!get("front_view_stream.count", text) || !parseUnsignedStrict(text, streamCount))
    {
        error = "invalid front_view_stream.count";
        return false;
    }
    envelope.frontViewStreams.clear();
    for (unsigned int idx = 0; idx < streamCount; ++idx)
    {
        const std::string prefix = "front_view_stream." + std::to_string(idx) + ".";
        ExternalIoFrontViewStreamRecord streamRecord;
        if (!get(prefix + "stream_id", streamRecord.streamId))
        {
            return false;
        }
        if (!get(prefix + "active_mode", streamRecord.activeMode) ||
            !get(prefix + "frame_id", streamRecord.frameId) ||
            !get(prefix + "sensor_type", streamRecord.sensorType) ||
            !get(prefix + "sequence", text) || !parseUnsignedStrict(text, streamRecord.sequence) ||
            !get(prefix + "timestamp_ms", text) || !parseUInt64Strict(text, streamRecord.timestampMs) ||
            !get(prefix + "frame_age_ms", text) || !parseDoubleStrict(text, streamRecord.frameAgeMs) ||
            !get(prefix + "latency_ms", text) || !parseDoubleStrict(text, streamRecord.latencyMs) ||
            !get(prefix + "confidence", text) || !parseDoubleStrict(text, streamRecord.confidence) ||
            !get(prefix + "stabilization_mode", streamRecord.stabilizationMode) ||
            !get(prefix + "stabilization_active", text) || !parseBoolStrict(text, streamRecord.stabilizationActive))
        {
            error = "invalid front_view_stream values at index " + std::to_string(idx);
            return false;
        }
        envelope.frontViewStreams.push_back(streamRecord);
    }

    optional("status.disqualified_sources", envelope.disqualifiedSources);
    optional("status.lockout_status", envelope.lockoutStatus);
    optional("status.auth_status", envelope.authStatus);
    optional("status.provenance_status", envelope.provenanceStatus);
    optional("status.logging_status", envelope.loggingStatus);
    optional("status.adapter_status", envelope.adapterStatus);
    optional("status.adapter_reason", envelope.adapterReason);
    optional("status.adapter_fields", envelope.adapterFields);

    if (envelope.mode.activeMode.empty())
    {
        error = "mode.active is required";
        return false;
    }
    if (envelope.frontView.streamCount > 0 && envelope.frontView.streamIndex >= envelope.frontView.streamCount)
    {
        error = "front_view.stream_index must be less than stream_count";
        return false;
    }
    if (!envelope.frontViewStreams.empty() &&
        envelope.frontView.streamCount > 0 &&
        envelope.frontView.streamCount != envelope.frontViewStreams.size())
    {
        error = "front_view.stream_count must match stream records";
        return false;
    }

    return true;
}
} // namespace

bool parseIoEnvelopeFormat(const std::string &text, IoEnvelopeFormat &format)
{
    const BuiltinCodec *codec = findCodecByName(text);
    if (codec == nullptr)
    {
        return false;
    }
    format = codec->format;
    return true;
}

std::vector<IoEnvelopeCodecDescriptor> listIoEnvelopeCodecs()
{
    std::vector<IoEnvelopeCodecDescriptor> codecs;
    const auto &registered = builtinCodecs();
    codecs.reserve(registered.size());
    for (const auto &codec : registered)
    {
        codecs.push_back({codec.canonicalName, codec.aliases});
    }
    return codecs;
}

bool isSupportedIoEnvelopeFormat(const std::string &text)
{
    IoEnvelopeFormat parsed = IoEnvelopeFormat::Json;
    return parseIoEnvelopeFormat(text, parsed);
}

std::string ioEnvelopeFormatName(IoEnvelopeFormat format)
{
    const BuiltinCodec *codec = findCodecByFormat(format);
    if (codec == nullptr)
    {
        return "unknown";
    }
    return codec->canonicalName;
}

IoEnvelopeParseResult parseExternalIoEnvelope(const std::string &formatName, const std::string &payload)
{
    IoEnvelopeFormat format = IoEnvelopeFormat::Json;
    if (!parseIoEnvelopeFormat(formatName, format))
    {
        IoEnvelopeParseResult result;
        result.error = "unsupported format: " + formatName;
        return result;
    }
    return parseExternalIoEnvelope(format, payload);
}

IoEnvelopeParseResult parseExternalIoEnvelope(IoEnvelopeFormat format, const std::string &payload)
{
    IoEnvelopeParseResult result;
    std::map<std::string, std::string> flat;

    if (format == IoEnvelopeFormat::KeyValue)
    {
        std::stringstream stream(payload);
        std::string line;
        int lineNumber = 0;
        while (std::getline(stream, line))
        {
            ++lineNumber;
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }
            if (line.empty() || line[0] == '#')
            {
                continue;
            }
            const std::size_t eq = line.find('=');
            if (eq == std::string::npos)
            {
                result.error = "kv missing '=' at line " + std::to_string(lineNumber);
                return result;
            }
            std::string value;
            if (!kvUnescape(line.substr(eq + 1), value))
            {
                result.error = "kv invalid escape at line " + std::to_string(lineNumber);
                return result;
            }
            const std::string key = line.substr(0, eq);
            if (flat.find(key) != flat.end())
            {
                result.error = "kv duplicate key: " + key;
                return result;
            }
            flat[key] = value;
        }
    }
    else
    {
        if (!parseFlatJson(payload, flat, result.error))
        {
            return result;
        }
    }

    if (!inflateEnvelope(flat, result.envelope, result.error))
    {
        return result;
    }
    result.ok = true;
    return result;
}

IoEnvelopeSerializeResult serializeExternalIoEnvelope(const std::string &formatName, const ExternalIoEnvelope &envelope)
{
    IoEnvelopeFormat format = IoEnvelopeFormat::Json;
    if (!parseIoEnvelopeFormat(formatName, format))
    {
        IoEnvelopeSerializeResult result;
        result.error = "unsupported format: " + formatName;
        return result;
    }
    return serializeExternalIoEnvelope(format, envelope);
}

IoEnvelopeSerializeResult serializeExternalIoEnvelope(IoEnvelopeFormat format, const ExternalIoEnvelope &envelope)
{
    IoEnvelopeSerializeResult result;
    std::map<std::string, std::string> flat;
    flattenEnvelope(envelope, flat);

    if (format == IoEnvelopeFormat::KeyValue)
    {
        std::ostringstream out;
        for (const auto &entry : flat)
        {
            out << entry.first << "=" << kvEscape(entry.second) << "\n";
        }
        result.ok = true;
        result.payload = out.str();
        return result;
    }

    std::ostringstream out;
    out << "{";
    bool first = true;
    for (const auto &entry : flat)
    {
        if (!first)
        {
            out << ",";
        }
        out << "\"" << jsonEscape(entry.first) << "\":\"" << jsonEscape(entry.second) << "\"";
        first = false;
    }
    out << "}";
    result.ok = true;
    result.payload = out.str();
    return result;
}

IoEnvelopeSerializeResult convertExternalIoEnvelope(
    const std::string &payload,
    const std::string &inputFormatName,
    const std::string &outputFormatName)
{
    IoEnvelopeFormat inputFormat = IoEnvelopeFormat::Json;
    IoEnvelopeFormat outputFormat = IoEnvelopeFormat::Json;
    if (!parseIoEnvelopeFormat(inputFormatName, inputFormat))
    {
        IoEnvelopeSerializeResult result;
        result.error = "unsupported input format: " + inputFormatName;
        return result;
    }
    if (!parseIoEnvelopeFormat(outputFormatName, outputFormat))
    {
        IoEnvelopeSerializeResult result;
        result.error = "unsupported output format: " + outputFormatName;
        return result;
    }
    return convertExternalIoEnvelope(payload, inputFormat, outputFormat);
}

IoEnvelopeSerializeResult convertExternalIoEnvelope(
    const std::string &payload,
    IoEnvelopeFormat inputFormat,
    IoEnvelopeFormat outputFormat)
{
    IoEnvelopeSerializeResult result;
    const IoEnvelopeParseResult parsed = parseExternalIoEnvelope(inputFormat, payload);
    if (!parsed.ok)
    {
        result.error = parsed.error;
        return result;
    }
    return serializeExternalIoEnvelope(outputFormat, parsed.envelope);
}
} // namespace tools
