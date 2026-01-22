#ifndef CORE_MODE_MANAGER_H
#define CORE_MODE_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "core/multi_modal_types.h"
#include "core/sensors.h"

struct TrendHistory
{
    std::vector<int> values{};
    std::size_t cursor = 0;
    int sum = 0;
};

enum class TrackingMode
{
    GpsIns,
    Gps,
    Vio,
    Lio,
    RadarInertial,
    Thermal,
    Radar,
    Vision,
    Lidar,
    MagBaro,
    Magnetometer,
    Baro,
    Celestial,
    DeadReckoning,
    Inertial,
    Hold
};

struct ModeDecision
{
    TrackingMode mode;
    std::string reason;
};

struct ModeManagerConfig
{
    int minHealthyCount = 2;
    int minDwellSteps = 3;
    double maxDataAgeSeconds = 1.0;
    double minConfidence = 0.0;
    int maxStaleCount = 0;
    int maxLowConfidenceCount = 0;
    int lockoutSteps = 0;
    int maxDisagreementCount = 0;
    double disagreementThreshold = 0.0;
    int historyWindow = 0;
    double maxResidualAgeSeconds = 0.5;
    std::vector<std::string> permittedSensors{};
    bool celestialAllowed = false;
    bool celestialDatasetAvailable = true;
    std::vector<std::string> ladderOrder = {
        "gps_ins",
        "gps",
        "vio",
        "lio",
        "radar_inertial",
        "vision",
        "lidar",
        "radar",
        "thermal",
        "mag_baro",
        "magnetometer",
        "baro",
        "celestial",
        "dead_reckoning",
        "imu",
        "hold"};
};

class ModeManager
{
public:
    explicit ModeManager(ModeManagerConfig config = {});
    ModeDecision decide(const std::vector<SensorBase *> &sensors);
    ModeDecisionDetail decideDetailed(const std::vector<SensorBase *> &sensors);
    static std::string modeName(TrackingMode mode);
    const ModeDecisionDetail &getLastDecisionDetail() const;

private:
    ModeManagerConfig config;
    TrackingMode currentMode = TrackingMode::Hold;
    int dwellSteps = 0;
    std::unordered_map<std::string, int> healthyCounts;
    std::unordered_map<std::string, int> staleCounts;
    std::unordered_map<std::string, int> lowConfidenceCounts;
    std::unordered_map<std::string, int> disagreementCounts;
    std::unordered_map<std::string, TrendHistory> staleHistory;
    std::unordered_map<std::string, TrendHistory> lowConfidenceHistory;
    std::unordered_map<std::string, TrendHistory> disagreementHistory;
    std::unordered_map<std::string, int> lockoutRemaining;
    ModeDecisionDetail lastDecisionDetail{};
};

#endif // CORE_MODE_MANAGER_H
