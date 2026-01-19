#ifndef CORE_MODE_MANAGER_H
#define CORE_MODE_MANAGER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "core/sensors.h"

enum class TrackingMode
{
    Gps,
    Thermal,
    Radar,
    Vision,
    Lidar,
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
    std::vector<std::string> permittedSensors{};
    bool celestialAllowed = false;
};

class ModeManager
{
public:
    explicit ModeManager(ModeManagerConfig config = {});
    ModeDecision decide(const std::vector<SensorBase *> &sensors);
    static std::string modeName(TrackingMode mode);

private:
    ModeManagerConfig config;
    TrackingMode currentMode = TrackingMode::Hold;
    int dwellSteps = 0;
    std::unordered_map<std::string, int> healthyCounts;
};

#endif // CORE_MODE_MANAGER_H
