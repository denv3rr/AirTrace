#ifndef UI_FRONT_VIEW_H
#define UI_FRONT_VIEW_H

#include <cstdint>
#include <random>
#include <string>
#include <vector>

#include "core/sim_config.h"

struct FrontViewFrameResult
{
    std::string activeMode;
    std::string viewState;
    std::string frameId;
    std::string sourceId;
    std::string sensorType;
    unsigned int sequence = 0;
    std::uint64_t timestampMs = 0;
    double frameAgeMs = 0.0;
    double acquisitionLatencyMs = 0.0;
    double processingLatencyMs = 0.0;
    double renderLatencyMs = 0.0;
    double latencyMs = 0.0;
    int droppedFrames = 0;
    std::string dropReason;
    bool spoofActive = false;
    double confidence = 0.0;
    std::string provenance;
    std::string authStatus;
    std::string streamId;
    unsigned int streamIndex = 0;
    unsigned int streamCount = 0;
    unsigned int maxConcurrentViews = 0;
    std::string stabilizationMode;
    bool stabilizationActive = false;
    double stabilizationErrorDeg = 0.0;
    double gimbalYawDeg = 0.0;
    double gimbalPitchDeg = 0.0;
    double gimbalYawRateDegPerSec = 0.0;
    double gimbalPitchRateDegPerSec = 0.0;
};

std::vector<std::string> frontViewSupportedModes();
bool frontViewModeSupported(const std::string &mode);
bool frontViewBuildCycleOrder(const SimConfig::FrontViewConfig &config, std::vector<std::string> &order, std::string &reason);
bool frontViewGenerateFrame(const SimConfig::FrontViewConfig &config,
                            const std::string &mode,
                            unsigned int sequence,
                            std::mt19937 &rng,
                            FrontViewFrameResult &result,
                            std::string &reason,
                            const std::string &streamId = "primary",
                            unsigned int streamIndex = 1,
                            unsigned int streamCount = 1);
bool frontViewCycleFrames(const SimConfig::FrontViewConfig &config,
                          bool cycleAllModes,
                          std::mt19937 &rng,
                          std::vector<FrontViewFrameResult> &frames,
                          std::string &reason);

#endif // UI_FRONT_VIEW_H
