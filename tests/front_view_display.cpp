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

    std::vector<FrontViewFrameResult> frames;
    bool cycleOk = frontViewCycleFrames(config, true, rng, frames, reason);
    assert(cycleOk);
    assert(reason == "ok");
    assert(frames.size() == config.autoCycleOrder.size());

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
}
