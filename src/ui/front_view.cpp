#include "ui/front_view.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <sstream>

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

bool isValidPattern(const std::string &pattern)
{
    return pattern == "gradient" || pattern == "hotspot" || pattern == "noise" || pattern == "sweep";
}

bool isValidMotion(const std::string &motion)
{
    return motion == "linear" || motion == "orbit" || motion == "jitter" || motion == "static";
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
                            std::string &reason)
{
    result = FrontViewFrameResult{};

    const std::string normalizedMode = toLower(mode);
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

    std::uniform_real_distribution<double> jitterDist(0.0, 20.0);
    std::uniform_real_distribution<double> confidenceNoise(-0.05, 0.05);

    result.activeMode = normalizedMode;
    result.viewState = viewStateForMode(normalizedMode);
    result.sensorType = sensorTypeForMode(normalizedMode);
    result.sequence = sequence;
    result.spoofActive = true;
    result.provenance = "simulation";
    result.authStatus = "authorized";

    std::ostringstream frameId;
    frameId << "fv_" << normalizedMode << "_" << std::setw(6) << std::setfill('0') << sequence;
    result.frameId = frameId.str();

    result.latencyMs = baseLatencyForMode(normalizedMode) + jitterDist(rng);
    double confidence = 1.0 - confidencePenaltyForPattern(config.spoofPattern) -
                        confidencePenaltyForMotion(config.spoofMotionProfile) +
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

    if (!cycleAllModes)
    {
        order = {order.front()};
    }

    unsigned int sequence = 1;
    for (const auto &mode : order)
    {
        FrontViewFrameResult frame;
        std::string frameReason;
        if (!frontViewGenerateFrame(config, mode, sequence, rng, frame, frameReason))
        {
            reason = frameReason;
            return false;
        }
        frames.push_back(frame);
        ++sequence;
    }

    reason = "ok";
    return true;
}
