#include "core/mode_manager.h"

ModeDecision ModeManager::decide(const std::vector<SensorBase *> &sensors) const
{
    auto findHealthy = [&](const std::string &name) -> SensorBase *
    {
        for (auto *sensor : sensors)
        {
            if (sensor && sensor->getName() == name && sensor->getStatus().healthy)
            {
                return sensor;
            }
        }
        return nullptr;
    };

    if (findHealthy("gps"))
    {
        return {TrackingMode::Gps, "gps_healthy"};
    }
    if (findHealthy("thermal"))
    {
        return {TrackingMode::Thermal, "thermal_healthy"};
    }
    if (findHealthy("radar"))
    {
        return {TrackingMode::Radar, "radar_healthy"};
    }
    if (findHealthy("dead_reckoning"))
    {
        return {TrackingMode::DeadReckoning, "dead_reckoning_only"};
    }
    if (findHealthy("imu"))
    {
        return {TrackingMode::Inertial, "imu_only"};
    }

    return {TrackingMode::Hold, "no_sensors"};
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
