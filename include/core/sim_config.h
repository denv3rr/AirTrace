#ifndef CORE_SIM_CONFIG_H
#define CORE_SIM_CONFIG_H

#include <string>
#include <vector>

#include "core/motion_models.h"
#include "core/sensors.h"
#include "core/state.h"

struct SimConfig
{
    State9 initialState{};
    double dt = 0.2;
    int steps = 20;
    unsigned int seed = 42;

    MotionBounds bounds{{-1000.0, -1000.0, 0.0}, {1000.0, 1000.0, 1000.0}, 250.0, 20.0, 12.0};
    ManeuverParams maneuvers{3.0, 0.35};

    SensorConfig gps{1.0, 2.0, 0.1, 0.03, 5000.0};
    SensorConfig thermal{5.0, 5.0, 0.15, 0.08, 1200.0};
    SensorConfig deadReckoning{20.0, 0.5, 0.0, 0.0, 0.0};
    SensorConfig imu{50.0, 0.2, 0.05, 0.0, 0.0};
    SensorConfig radar{2.0, 1.0, 0.08, 0.02, 2000.0};
};

struct ConfigIssue
{
    std::string key;
    std::string message;
};

struct ConfigResult
{
    SimConfig config;
    std::vector<ConfigIssue> issues;
    bool ok = true;
};

ConfigResult loadSimConfig(const std::string &path);

#endif // CORE_SIM_CONFIG_H
