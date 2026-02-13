#include "ui/front_view.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <unordered_set>

namespace
{
std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch)
    {
        return static_cast<char>(std::tolower(ch));
    });
    return value;
}

bool isValidStreamId(const std::string &value)
{
    if (value.empty())
    {
        return false;
    }
    for (char ch : value)
    {
        const unsigned char uch = static_cast<unsigned char>(ch);
        if (!(std::islower(uch) || std::isdigit(uch) || ch == '_' || ch == '-'))
        {
            return false;
        }
    }
    return true;
}

bool isValidStabilizationMode(const std::string &value)
{
    return value == "off" ||
           value == "eis" ||
           value == "gimbal_lock" ||
           value == "hybrid";
}

std::string sensorTypeForMode(const std::string &mode)
{
    if (mode == "eo_gray")
    {
        return "camera";
    }
    if (mode == "ir_white_hot" || mode == "ir_black_hot" || mode == "ir_false_color")
    {
        return "thermal";
    }
    if (mode == "fusion_overlay")
    {
        return "fusion";
    }
    if (mode == "proximity_3d")
    {
        return "point_cloud";
    }
    return "proximity";
}

std::string viewStateForMode(const std::string &mode)
{
    if (mode == "proximity_3d")
    {
        return "3d_stream";
    }
    if (mode == "proximity_2d")
    {
        return "2d_map";
    }
    return "front_view";
}

double baseLatencyForMode(const std::string &mode)
{
    if (mode == "proximity_3d")
    {
        return 70.0;
    }
    if (mode == "fusion_overlay")
    {
        return 40.0;
    }
    if (mode == "ir_false_color")
    {
        return 35.0;
    }
    return 25.0;
}

double confidencePenaltyForPattern(const std::string &pattern)
{
    if (pattern == "noise")
    {
        return 0.25;
    }
    if (pattern == "sweep")
    {
        return 0.10;
    }
    if (pattern == "hotspot")
    {
        return 0.05;
    }
    return 0.0;
}

double confidencePenaltyForMotion(const std::string &motion)
{
    if (motion == "jitter")
    {
        return 0.20;
    }
    if (motion == "orbit")
    {
        return 0.10;
    }
    return 0.0;
}

double stabilizationFactorForMode(const std::string &mode)
{
    if (mode == "gimbal_lock")
    {
        return 0.35;
    }
    if (mode == "hybrid")
    {
        return 0.30;
    }
    if (mode == "eis")
    {
        return 0.55;
    }
    return 1.0;
}

double stabilizationErrorForMotion(const std::string &motion)
{
    if (motion == "jitter")
    {
        return 2.8;
    }
    if (motion == "orbit")
    {
        return 1.5;
    }
    if (motion == "linear")
    {
        return 0.9;
    }
    return 0.3;
}

bool isValidPattern(const std::string &pattern)
{
    return pattern == "gradient" || pattern == "hotspot" || pattern == "noise" || pattern == "sweep";
}

bool isValidMotion(const std::string &motion)
{
    return motion == "linear" || motion == "orbit" || motion == "jitter" || motion == "static";
}

bool resolveStreamIds(const SimConfig::FrontViewConfig &config, std::vector<std::string> &streamIds, std::string &reason)
{
    streamIds.clear();
    if (config.maxConcurrentViews < 1)
    {
        reason = "front_view_stream_invalid";
        return false;
    }
    if (config.streamIds.empty())
    {
        reason = "front_view_stream_invalid";
        return false;
    }

    const std::size_t requested = static_cast<std::size_t>(config.maxConcurrentViews);
    const std::size_t count = std::min(config.streamIds.size(), requested);
    std::unordered_set<std::string> unique;
    for (std::size_t idx = 0; idx < count; ++idx)
    {
        const std::string normalized = toLower(config.streamIds[idx]);
        if (!isValidStreamId(normalized))
        {
            reason = "front_view_stream_invalid";
            return false;
        }
        if (!unique.insert(normalized).second)
        {
            reason = "front_view_stream_invalid";
            return false;
        }
        streamIds.push_back(normalized);
    }
    if (streamIds.empty())
    {
        reason = "front_view_stream_invalid";
        return false;
    }
    reason = "ok";
    return true;
}
} // namespace

std::vector<std::string> frontViewSupportedModes()
{
    return {"eo_gray", "ir_white_hot", "ir_black_hot", "ir_false_color", "fusion_overlay", "proximity_2d", "proximity_3d"};
}

bool frontViewModeSupported(const std::string &mode)
{
    const std::string lowered = toLower(mode);
    for (const auto &entry : frontViewSupportedModes())
    {
        if (entry == lowered)
        {
            return true;
        }
    }
    return false;
}

bool frontViewBuildCycleOrder(const SimConfig::FrontViewConfig &config, std::vector<std::string> &order, std::string &reason)
{
    order.clear();
    if (config.autoCycleEnabled)
    {
        order = config.autoCycleOrder;
    }
    else
    {
        order = config.displayFamilies;
    }
    if (order.empty())
    {
        reason = "front_view_cycle_empty";
        return false;
    }
    for (auto &mode : order)
    {
        mode = toLower(mode);
        if (!frontViewModeSupported(mode))
        {
            reason = "front_view_mode_invalid";
            return false;
        }
    }
    reason = "ok";
    return true;
}

bool frontViewGenerateFrame(const SimConfig::FrontViewConfig &config,
                            const std::string &mode,
                            unsigned int sequence,
                            std::mt19937 &rng,
                            FrontViewFrameResult &result,
                            std::string &reason,
                            const std::string &streamId,
                            unsigned int streamIndex,
                            unsigned int streamCount)
{
    result = FrontViewFrameResult{};

    const std::string normalizedMode = toLower(mode);
    const std::string normalizedStreamId = toLower(streamId);
    const std::string stabilizationMode = toLower(config.stabilizationMode);
    if (!config.enabled)
    {
        reason = "front_view_disabled";
        return false;
    }
    if (!config.spoofEnabled)
    {
        reason = "front_view_sensor_unavailable";
        return false;
    }
    if (!frontViewModeSupported(normalizedMode))
    {
        reason = "front_view_mode_invalid";
        return false;
    }
    if (!isValidPattern(config.spoofPattern) || !isValidMotion(config.spoofMotionProfile))
    {
        reason = "front_view_spoof_invalid";
        return false;
    }
    if (config.spoofRateHz <= 0.0 || config.spoofRateHz > 240.0)
    {
        reason = "front_view_spoof_rate_invalid";
        return false;
    }
    if (config.frameMaxAgeMs <= 0.0 || config.frameMaxAgeMs > 10000.0)
    {
        reason = "front_view_frame_contract_invalid";
        return false;
    }
    if (config.frameMinConfidence < 0.0 || config.frameMinConfidence > 1.0)
    {
        reason = "front_view_frame_contract_invalid";
        return false;
    }
    if (config.maxConcurrentViews < 1 ||
        !isValidStreamId(normalizedStreamId) ||
        streamCount == 0 ||
        streamIndex == 0 ||
        streamIndex > streamCount ||
        streamCount > static_cast<unsigned int>(config.maxConcurrentViews))
    {
        reason = "front_view_stream_invalid";
        return false;
    }
    if (!isValidStabilizationMode(stabilizationMode))
    {
        reason = "front_view_stabilization_invalid";
        return false;
    }
    if (config.stabilizationEnabled && stabilizationMode == "off")
    {
        reason = "front_view_stabilization_invalid";
        return false;
    }
    if (!config.stabilizationEnabled && stabilizationMode != "off")
    {
        reason = "front_view_stabilization_invalid";
        return false;
    }
    if (config.gimbalEnabled &&
        (config.gimbalMaxYawRateDegPerSec <= 0.0 || config.gimbalMaxPitchRateDegPerSec <= 0.0))
    {
        reason = "front_view_gimbal_invalid";
        return false;
    }
    if (config.gimbalEnabled && !config.stabilizationEnabled)
    {
        reason = "front_view_gimbal_invalid";
        return false;
    }

    std::uniform_real_distribution<double> latencyJitter(0.0, 8.0);
    std::uniform_real_distribution<double> ageJitter(0.0, 12.0);
    std::uniform_real_distribution<double> confidenceNoise(-0.04, 0.04);
    std::uniform_real_distribution<double> stabilizationNoise(0.0, 0.35);
    std::uniform_real_distribution<double> gimbalNoise(-3.0, 3.0);

    result.activeMode = normalizedMode;
    result.viewState = viewStateForMode(normalizedMode);
    result.sensorType = sensorTypeForMode(normalizedMode);
    result.sequence = sequence;
    result.spoofActive = true;
    result.provenance = "simulation";
    result.authStatus = "authorized";
    result.streamId = normalizedStreamId;
    result.streamIndex = streamIndex;
    result.streamCount = streamCount;
    result.maxConcurrentViews = static_cast<unsigned int>(config.maxConcurrentViews);

    std::ostringstream frameId;
    frameId << "fv_" << normalizedStreamId << "_" << normalizedMode << "_" << std::setw(6) << std::setfill('0') << sequence;
    result.frameId = frameId.str();
    result.sourceId = normalizedStreamId + "_" + result.sensorType;

    const double frameIntervalMs = 1000.0 / config.spoofRateHz;
    const std::uint64_t timestamp = static_cast<std::uint64_t>(std::llround((static_cast<double>(sequence) - 1.0) * frameIntervalMs));
    result.timestampMs = timestamp;

    const double baseLatency = baseLatencyForMode(normalizedMode);
    result.acquisitionLatencyMs = 2.0 + (0.15 * baseLatency) + (0.2 * latencyJitter(rng));
    result.processingLatencyMs = (0.45 * baseLatency) + (0.55 * latencyJitter(rng));
    result.renderLatencyMs = (0.40 * baseLatency) + (0.35 * latencyJitter(rng));
    result.latencyMs = result.acquisitionLatencyMs + result.processingLatencyMs + result.renderLatencyMs;

    const double streamAgePenalty = static_cast<double>(streamCount > 1 ? (streamCount - 1) : 0) * 3.0;
    result.frameAgeMs = result.latencyMs + streamAgePenalty + ageJitter(rng);
    if (config.spoofMotionProfile == "jitter")
    {
        result.frameAgeMs += 6.0;
    }

    result.stabilizationMode = stabilizationMode;
    double stabilizationErrorDeg = stabilizationErrorForMotion(config.spoofMotionProfile) + stabilizationNoise(rng);
    if (config.stabilizationEnabled)
    {
        stabilizationErrorDeg *= stabilizationFactorForMode(stabilizationMode);
    }
    else
    {
        stabilizationErrorDeg += 1.5;
    }
    result.stabilizationErrorDeg = stabilizationErrorDeg;
    result.stabilizationActive = config.stabilizationEnabled && (stabilizationErrorDeg <= 3.0);

    if (config.gimbalEnabled)
    {
        result.gimbalYawRateDegPerSec = (0.55 * config.gimbalMaxYawRateDegPerSec) + latencyJitter(rng) + gimbalNoise(rng);
        result.gimbalPitchRateDegPerSec = (0.55 * config.gimbalMaxPitchRateDegPerSec) + (0.5 * latencyJitter(rng)) + gimbalNoise(rng);
        if (result.gimbalYawRateDegPerSec > config.gimbalMaxYawRateDegPerSec ||
            result.gimbalPitchRateDegPerSec > config.gimbalMaxPitchRateDegPerSec)
        {
            reason = "front_view_gimbal_invalid";
            return false;
        }
        const double yawStep = static_cast<double>(sequence) * 12.0 + static_cast<double>(streamIndex) * 15.0;
        result.gimbalYawDeg = std::fmod(yawStep, 360.0) - 180.0;
        const double pitchRadians = (static_cast<double>(sequence) + static_cast<double>(streamIndex)) * 0.25;
        result.gimbalPitchDeg = std::sin(pitchRadians) * 45.0;
    }

    double confidence = 1.0 -
                        confidencePenaltyForPattern(config.spoofPattern) -
                        confidencePenaltyForMotion(config.spoofMotionProfile) -
                        (0.015 * static_cast<double>(streamCount > 1 ? (streamCount - 1) : 0)) -
                        (result.stabilizationErrorDeg / 30.0) +
                        confidenceNoise(rng);
    if (confidence < 0.0)
    {
        confidence = 0.0;
    }
    if (confidence > 1.0)
    {
        confidence = 1.0;
    }
    result.confidence = confidence;

    if (result.frameAgeMs > config.frameMaxAgeMs)
    {
        reason = "front_view_frame_stale";
        return false;
    }
    if (result.confidence < config.frameMinConfidence)
    {
        reason = "front_view_confidence_low";
        return false;
    }

    if (result.latencyMs > config.renderLatencyBudgetMs)
    {
        result.droppedFrames = 1;
        result.dropReason = "render_latency_exceeded";
    }
    else
    {
        result.droppedFrames = 0;
        result.dropReason.clear();
    }

    reason = "ok";
    return true;
}

bool frontViewCycleFrames(const SimConfig::FrontViewConfig &config,
                          bool cycleAllModes,
                          std::mt19937 &rng,
                          std::vector<FrontViewFrameResult> &frames,
                          std::string &reason)
{
    frames.clear();

    std::vector<std::string> order;
    if (!frontViewBuildCycleOrder(config, order, reason))
    {
        return false;
    }

    std::vector<std::string> streamIds;
    if (!resolveStreamIds(config, streamIds, reason))
    {
        return false;
    }

    if (!cycleAllModes)
    {
        order = {order.front()};
    }

    unsigned int sequence = 1;
    const unsigned int streamCount = static_cast<unsigned int>(streamIds.size());
    for (const auto &mode : order)
    {
        for (unsigned int streamIndex = 0; streamIndex < streamCount; ++streamIndex)
        {
            FrontViewFrameResult frame;
            std::string frameReason;
            if (!frontViewGenerateFrame(config,
                                        mode,
                                        sequence,
                                        rng,
                                        frame,
                                        frameReason,
                                        streamIds[streamIndex],
                                        streamIndex + 1,
                                        streamCount))
            {
                reason = frameReason;
                return false;
            }
            frames.push_back(frame);
            ++sequence;
        }
    }

    reason = "ok";
    return true;
}
