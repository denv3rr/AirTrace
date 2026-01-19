#include "core/mode_manager.h"

namespace
{
const std::vector<std::string> kSensorPriority = {
    "gps",
    "vision",
    "lidar",
    "radar",
    "thermal",
    "magnetometer",
    "baro",
    "celestial",
    "dead_reckoning",
    "imu"};

std::string sensorNameForMode(TrackingMode mode)
{
    switch (mode)
    {
    case TrackingMode::Gps:
        return "gps";
    case TrackingMode::Thermal:
        return "thermal";
    case TrackingMode::Radar:
        return "radar";
    case TrackingMode::Vision:
        return "vision";
    case TrackingMode::Lidar:
        return "lidar";
    case TrackingMode::Magnetometer:
        return "magnetometer";
    case TrackingMode::Baro:
        return "baro";
    case TrackingMode::Celestial:
        return "celestial";
    case TrackingMode::DeadReckoning:
        return "dead_reckoning";
    case TrackingMode::Inertial:
        return "imu";
    default:
        return "";
    }
}
} // namespace

ModeManager::ModeManager(ModeManagerConfig config)
    : config(config)
{
}

ModeDecision ModeManager::decide(const std::vector<SensorBase *> &sensors)
{
    auto permitted = [&](const std::string &name) -> bool
    {
        if (name == "celestial" && !config.celestialAllowed)
        {
            return false;
        }
        if (config.permittedSensors.empty())
        {
            return true;
        }
        for (const auto &allowed : config.permittedSensors)
        {
            if (allowed == name)
            {
                return true;
            }
        }
        return false;
    };

    auto updateHealthCount = [&](const std::string &name)
    {
        bool healthy = false;
        for (auto *sensor : sensors)
        {
            if (sensor && sensor->getName() == name)
            {
                healthy = sensor->getStatus().healthy;
                break;
            }
        }
        int &count = healthyCounts[name];
        count = healthy ? (count + 1) : 0;
    };

    for (const auto &name : kSensorPriority)
    {
        updateHealthCount(name);
    }

    auto eligible = [&](const std::string &name) -> bool
    {
        if (!permitted(name))
        {
            return false;
        }
        auto it = healthyCounts.find(name);
        if (it == healthyCounts.end())
        {
            return false;
        }
        return it->second >= config.minHealthyCount;
    };

    TrackingMode desiredMode = TrackingMode::Hold;
    std::string desiredReason = "no_sensors";
    for (const auto &name : kSensorPriority)
    {
        if (eligible(name))
        {
            desiredReason = name + "_healthy";
            if (name == "gps")
            {
                desiredMode = TrackingMode::Gps;
                if (eligible("imu"))
                {
                    desiredReason = "gps_ins_healthy";
                }
            }
            else if (name == "vision")
            {
                desiredMode = TrackingMode::Vision;
            }
            else if (name == "lidar")
            {
                desiredMode = TrackingMode::Lidar;
            }
            else if (name == "thermal")
            {
                desiredMode = TrackingMode::Thermal;
            }
            else if (name == "radar")
            {
                desiredMode = TrackingMode::Radar;
            }
            else if (name == "magnetometer")
            {
                desiredMode = TrackingMode::Magnetometer;
            }
            else if (name == "baro")
            {
                desiredMode = TrackingMode::Baro;
            }
            else if (name == "celestial")
            {
                desiredMode = TrackingMode::Celestial;
            }
            else if (name == "dead_reckoning")
            {
                desiredMode = TrackingMode::DeadReckoning;
            }
            else
            {
                desiredMode = TrackingMode::Inertial;
            }
            break;
        }
    }

    if (desiredMode == TrackingMode::Hold)
    {
        currentMode = TrackingMode::Hold;
        dwellSteps = 0;
        return {TrackingMode::Hold, "no_sensors"};
    }

    if (currentMode == TrackingMode::Hold)
    {
        currentMode = desiredMode;
        dwellSteps = 0;
        return {currentMode, "enter_" + modeName(currentMode)};
    }

    std::string currentSensor = sensorNameForMode(currentMode);
    bool currentEligible = !currentSensor.empty() && eligible(currentSensor);

    if (currentMode == desiredMode)
    {
        dwellSteps += 1;
        return {currentMode, "maintain_" + modeName(currentMode)};
    }

    if (!currentEligible)
    {
        currentMode = desiredMode;
        dwellSteps = 0;
        return {currentMode, "switch_unhealthy_" + modeName(currentMode)};
    }

    if (dwellSteps < config.minDwellSteps)
    {
        dwellSteps += 1;
        return {currentMode, "dwell_" + modeName(currentMode)};
    }

    currentMode = desiredMode;
    dwellSteps = 0;
    return {currentMode, "switch_" + modeName(currentMode)};
}

std::string ModeManager::modeName(TrackingMode mode)
{
    switch (mode)
    {
    case TrackingMode::Gps:
        return "gps";
    case TrackingMode::Thermal:
        return "thermal";
    case TrackingMode::Radar:
        return "radar";
    case TrackingMode::Vision:
        return "vision";
    case TrackingMode::Lidar:
        return "lidar";
    case TrackingMode::Magnetometer:
        return "magnetometer";
    case TrackingMode::Baro:
        return "baro";
    case TrackingMode::Celestial:
        return "celestial";
    case TrackingMode::DeadReckoning:
        return "dead_reckoning";
    case TrackingMode::Inertial:
        return "inertial";
    case TrackingMode::Hold:
        return "hold";
    default:
        return "unknown";
    }
}
