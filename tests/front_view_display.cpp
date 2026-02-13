#include "ui/front_view.h"

#include <cassert>
#include <random>
#include <string>
#include <vector>

void runFrontViewDisplayTests()
{
    SimConfig::FrontViewConfig config;
    config.enabled = true;
    config.displayFamilies = {"eo_gray", "ir_white_hot", "proximity_2d"};
    config.autoCycleEnabled = true;
    config.autoCycleOrder = {"eo_gray", "ir_white_hot", "proximity_2d"};
    config.renderLatencyBudgetMs = 120.0;
    config.frameMaxAgeMs = 500.0;
    config.frameMinConfidence = 0.25;
    config.maxConcurrentViews = 2;
    config.streamIds = {"primary", "turret"};
    config.stabilizationEnabled = true;
    config.stabilizationMode = "gimbal_lock";
    config.gimbalEnabled = true;
    config.gimbalMaxYawRateDegPerSec = 220.0;
    config.gimbalMaxPitchRateDegPerSec = 160.0;
    config.spoofEnabled = true;
    config.spoofPattern = "gradient";
    config.spoofMotionProfile = "linear";
    config.spoofRateHz = 15.0;

    std::vector<std::string> order;
    std::string reason;
    bool orderOk = frontViewBuildCycleOrder(config, order, reason);
    assert(orderOk);
    assert(reason == "ok");
    assert(order.size() == 3);

    std::mt19937 rng(1337U);
    FrontViewFrameResult frame;
    bool frameOk = frontViewGenerateFrame(config, "eo_gray", 1U, rng, frame, reason);
    assert(frameOk);
    assert(reason == "ok");
    assert(frame.activeMode == "eo_gray");
    assert(frame.sequence == 1U);
    assert(frame.spoofActive);
    assert(frame.provenance == "simulation");
    assert(frame.authStatus == "authorized");
    assert(frame.timestampMs == 0U);
    assert(frame.frameAgeMs >= 0.0);
    assert(frame.sourceId.find("camera") != std::string::npos);
    assert(frame.streamId == "primary");
    assert(frame.streamIndex == 1U);
    assert(frame.streamCount == 1U);
    assert(frame.stabilizationMode == "gimbal_lock");
    assert(frame.gimbalYawRateDegPerSec > 0.0);

    std::vector<FrontViewFrameResult> frames;
    bool cycleOk = frontViewCycleFrames(config, true, rng, frames, reason);
    assert(cycleOk);
    assert(reason == "ok");
    assert(frames.size() == config.autoCycleOrder.size() * static_cast<size_t>(config.maxConcurrentViews));
    assert(frames.front().streamId == "primary");
    assert(frames[1].streamId == "turret");

    SimConfig::FrontViewConfig invalidModeConfig = config;
    invalidModeConfig.autoCycleEnabled = false;
    invalidModeConfig.displayFamilies = {"invalid_mode"};
    std::vector<std::string> invalidOrder;
    bool invalidOrderOk = frontViewBuildCycleOrder(invalidModeConfig, invalidOrder, reason);
    assert(!invalidOrderOk);
    assert(reason == "front_view_mode_invalid");

    SimConfig::FrontViewConfig spoofDisabledConfig = config;
    spoofDisabledConfig.spoofEnabled = false;
    bool spoofDisabledOk = frontViewGenerateFrame(spoofDisabledConfig, "eo_gray", 1U, rng, frame, reason);
    assert(!spoofDisabledOk);
    assert(reason == "front_view_sensor_unavailable");

    SimConfig::FrontViewConfig staleConfig = config;
    staleConfig.frameMaxAgeMs = 1.0;
    bool staleOk = frontViewGenerateFrame(staleConfig, "eo_gray", 1U, rng, frame, reason);
    assert(!staleOk);
    assert(reason == "front_view_frame_stale");

    SimConfig::FrontViewConfig confidenceConfig = config;
    confidenceConfig.frameMinConfidence = 0.99;
    confidenceConfig.spoofPattern = "noise";
    confidenceConfig.spoofMotionProfile = "jitter";
    bool confidenceOk = frontViewGenerateFrame(confidenceConfig, "eo_gray", 1U, rng, frame, reason);
    assert(!confidenceOk);
    assert(reason == "front_view_confidence_low");
}
