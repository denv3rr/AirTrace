#ifndef UI_FRONT_VIEW_H
#define UI_FRONT_VIEW_H

#include <random>
#include <string>
#include <vector>

#include "core/sim_config.h"

struct FrontViewFrameResult
{
    std::string activeMode;
    std::string viewState;
    std::string frameId;
    std::string sensorType;
    unsigned int sequence = 0;
    double latencyMs = 0.0;
    int droppedFrames = 0;
    std::string dropReason;
    bool spoofActive = false;
    double confidence = 0.0;
    std::string provenance;
    std::string authStatus;
};

std::vector<std::string> frontViewSupportedModes();
bool frontViewModeSupported(const std::string &mode);
bool frontViewBuildCycleOrder(const SimConfig::FrontViewConfig &config, std::vector<std::string> &order, std::string &reason);
bool frontViewGenerateFrame(const SimConfig::FrontViewConfig &config,
                            const std::string &mode,
                            unsigned int sequence,
                            std::mt19937 &rng,
                            FrontViewFrameResult &result,
                            std::string &reason);
bool frontViewCycleFrames(const SimConfig::FrontViewConfig &config,
                          bool cycleAllModes,
                          std::mt19937 &rng,
                          std::vector<FrontViewFrameResult> &frames,
                          std::string &reason);

#endif // UI_FRONT_VIEW_H
