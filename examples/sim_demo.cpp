#include <iostream>
#include <random>
#include <vector>

#include "core/mode_manager.h"
#include "core/motion_models.h"
#include "core/sensors.h"
#include "core/sim_config.h"
#include "core/state.h"

namespace
{
MotionModelType cycleModel(int step)
{
    switch (step % 4)
    {
    case 0:
        return MotionModelType::ConstantVelocity;
    case 1:
        return MotionModelType::ConstantAcceleration;
    case 2:
        return MotionModelType::CoordinatedTurn;
    default:
        return MotionModelType::RandomManeuver;
    }
}
} // namespace

int main(int argc, char **argv)
{
    std::string configPath = "configs/sim_default.cfg";
    if (argc > 1)
    {
        configPath = argv[1];
    }

    ConfigResult loaded = loadSimConfig(configPath);
    if (!loaded.ok)
    {
        std::cerr << "Config issues:\n";
        for (const auto &issue : loaded.issues)
        {
            std::cerr << "- " << issue.key << ": " << issue.message << "\n";
        }
        return 1;
    }

    const SimConfig &cfg = loaded.config;
    std::mt19937 rng(cfg.seed);
    State9 state = cfg.initialState;

    MotionBounds bounds = cfg.bounds;
    ManeuverParams maneuvers = cfg.maneuvers;

    SensorConfig gpsCfg = cfg.gps;
    SensorConfig thermalCfg = cfg.thermal;
    SensorConfig drCfg = cfg.deadReckoning;
    SensorConfig imuCfg = cfg.imu;
    SensorConfig radarCfg = cfg.radar;

    GpsSensor gps(gpsCfg);
    ThermalSensor thermal(thermalCfg);
    DeadReckoningSensor deadReckoning(drCfg);
    ImuSensor imu(imuCfg);
    RadarSensor radar(radarCfg);
    std::vector<SensorBase *> sensors{&gps, &thermal, &radar, &deadReckoning, &imu};

    ModeManager modeManager;

    double dt = cfg.dt;
    for (int i = 0; i < cfg.steps; ++i)
    {
        MotionModelType model = cycleModel(i);
        state = stepMotionModel(state, model, dt, bounds, maneuvers, rng);

        Measurement gpsMeas = gps.sample(state, dt, rng);
        Measurement thermMeas = thermal.sample(state, dt, rng);
        Measurement radarMeas = radar.sample(state, dt, rng);
        Measurement drMeas = deadReckoning.sample(state, dt, rng);
        Measurement imuMeas = imu.sample(state, dt, rng);

        ModeDecision decision = modeManager.decide(sensors);
        Projection2D xy = projectXY(state);
        Projection2D xz = projectXZ(state);

        std::cout << "Step " << i << " | model=" << static_cast<int>(model)
                  << " | mode=" << ModeManager::modeName(decision.mode)
                  << " | pos=(" << state.position.x << ", " << state.position.y << ", " << state.position.z << ")"
                  << " | proj " << xy.plane << "=(" << xy.x << ", " << xy.y << ")"
                  << " | proj " << xz.plane << "=(" << xz.x << ", " << xz.y << ")";

        if (gpsMeas.valid && gpsMeas.position)
        {
            std::cout << " | gps=(" << gpsMeas.position->x << ", " << gpsMeas.position->y << ", " << gpsMeas.position->z << ")";
        }
        if (thermMeas.valid && thermMeas.position)
        {
            std::cout << " | thermal=(" << thermMeas.position->x << ", " << thermMeas.position->y << ", " << thermMeas.position->z << ")";
        }
        if (radarMeas.valid && radarMeas.range)
        {
            std::cout << " | radar_range=" << *radarMeas.range;
        }
        if (imuMeas.valid && imuMeas.velocity)
        {
            std::cout << " | imu_v=(" << imuMeas.velocity->x << ", " << imuMeas.velocity->y << ", " << imuMeas.velocity->z << ")";
        }
        if (drMeas.valid && drMeas.position)
        {
            std::cout << " | dr=(" << drMeas.position->x << ", " << drMeas.position->y << ", " << drMeas.position->z << ")";
        }
        if (!decision.reason.empty())
        {
            std::cout << " | decision=" << decision.reason;
        }
        std::cout << "\n";
    }

    std::cout << "Simulation complete.\n";
    return 0;
}
