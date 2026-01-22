#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>

#include "core/hash.h"
#include "core/mode_manager.h"
#include "core/mode_scheduler.h"
#include "core/motion_models.h"
#include "core/sensors.h"
#include "core/sim_config.h"
#include "core/state.h"

namespace
{
struct ConfigPathResult
{
    std::string path;
    std::vector<std::string> tried;
};

struct DatasetCheckResult
{
    bool ok = true;
    std::string message;
};

ConfigPathResult resolveConfigPath(int argc, char **argv)
{
    const std::string defaultPath = "configs/sim_default.cfg";
    std::string requested = (argc > 1) ? argv[1] : defaultPath;
    std::vector<std::string> tried;

    auto addCandidate = [&](const std::filesystem::path &candidate)
    {
        std::string value = candidate.string();
        for (const auto &existing : tried)
        {
            if (existing == value)
            {
                return;
            }
        }
        tried.push_back(value);
    };

    std::filesystem::path requestedPath(requested);
    addCandidate(requestedPath);
    addCandidate(std::filesystem::current_path() / requestedPath);

    std::filesystem::path exePath(argv[0]);
    std::filesystem::path exeDir = exePath.has_parent_path() ? exePath.parent_path() : std::filesystem::path{};
    if (!exeDir.empty())
    {
        addCandidate(exeDir / requestedPath);
        addCandidate(exeDir / ".." / requestedPath);
    }

    for (const auto &candidate : tried)
    {
        if (std::filesystem::exists(candidate))
        {
            return {candidate, tried};
        }
    }

    return {requested, tried};
}

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

std::vector<unsigned char> readFileBytes(const std::string &path, std::string &error)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        error = "unable to open";
        return {};
    }
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return data;
}

bool hasPermission(const SimConfig::PolicyConfig &policy, const std::string &permission)
{
    auto it = policy.rolePermissions.find(policy.activeRole);
    if (it == policy.rolePermissions.end())
    {
        return false;
    }
    for (const auto &entry : it->second)
    {
        if (entry == permission)
        {
            return true;
        }
    }
    return false;
}

DatasetCheckResult validateCelestialDataset(const SimConfig &cfg)
{
    DatasetCheckResult result;
    if (!cfg.dataset.celestialAvailable())
    {
        result.ok = false;
        result.message = "celestial dataset config incomplete";
        return result;
    }

    if (cfg.dataset.tier != SimConfig::DatasetTier::Minimal && !hasPermission(cfg.policy, "dataset_tier"))
    {
        result.ok = false;
        result.message = "dataset tier override not permitted";
        return result;
    }

    const std::string catalogPath = cfg.dataset.celestialCatalogPath;
    const std::string ephPath = cfg.dataset.celestialEphemerisPath;
    std::string error;

    std::vector<unsigned char> catalog = readFileBytes(catalogPath, error);
    if (catalog.empty())
    {
        result.ok = false;
        result.message = "catalog " + error;
        return result;
    }
    std::vector<unsigned char> ephemeris = readFileBytes(ephPath, error);
    if (ephemeris.empty())
    {
        result.ok = false;
        result.message = "ephemeris " + error;
        return result;
    }

    if (cfg.dataset.maxSizeMB > 0.0)
    {
        double totalSizeMB = (static_cast<double>(catalog.size()) + static_cast<double>(ephemeris.size())) / (1024.0 * 1024.0);
        if (totalSizeMB > cfg.dataset.maxSizeMB)
        {
            result.ok = false;
            result.message = "dataset size exceeds limit";
            return result;
        }
    }

    std::string catalogHash = sha256Hex(catalog);
    if (!hashEquals(cfg.dataset.celestialCatalogHash, catalogHash))
    {
        result.ok = false;
        result.message = "catalog hash mismatch";
        return result;
    }
    std::string ephemerisHash = sha256Hex(ephemeris);
    if (!hashEquals(cfg.dataset.celestialEphemerisHash, ephemerisHash))
    {
        result.ok = false;
        result.message = "ephemeris hash mismatch";
        return result;
    }

    return result;
}
} // namespace

int main(int argc, char **argv)
{
    ConfigPathResult config = resolveConfigPath(argc, argv);

    ConfigResult loaded = loadSimConfig(config.path);
    if (!loaded.ok)
    {
        std::cerr << "Config issues:\n";
        for (const auto &issue : loaded.issues)
        {
            std::cerr << "- " << issue.key << ": " << issue.message << "\n";
        }
        if (!config.tried.empty())
        {
            std::cerr << "Config search paths:\n";
            for (const auto &candidate : config.tried)
            {
                std::cerr << "- " << candidate << "\n";
            }
        }
        return 1;
    }

    const SimConfig &cfg = loaded.config;
    std::mt19937 rng(cfg.seed);
    State9 state = cfg.initialState;

    bool celestialAllowed = false;
    bool celestialDatasetAvailable = false;
    if (cfg.dataset.celestialAvailable())
    {
        DatasetCheckResult datasetCheck = validateCelestialDataset(cfg);
        if (!datasetCheck.ok)
        {
            std::cerr << "Celestial dataset invalid: " << datasetCheck.message << "\n";
            return 1;
        }
        celestialAllowed = true;
        celestialDatasetAvailable = true;
    }

    MotionBounds bounds = cfg.bounds;
    ManeuverParams maneuvers = cfg.maneuvers;

    SensorConfig gpsCfg = cfg.gps;
    SensorConfig thermalCfg = cfg.thermal;
    SensorConfig drCfg = cfg.deadReckoning;
    SensorConfig imuCfg = cfg.imu;
    SensorConfig radarCfg = cfg.radar;
    SensorConfig visionCfg = cfg.vision;
    SensorConfig lidarCfg = cfg.lidar;
    SensorConfig magnetometerCfg = cfg.magnetometer;
    SensorConfig baroCfg = cfg.baro;
    SensorConfig celestialCfg = cfg.celestial;

    GpsSensor gps(gpsCfg);
    ThermalSensor thermal(thermalCfg);
    DeadReckoningSensor deadReckoning(drCfg);
    ImuSensor imu(imuCfg);
    RadarSensor radar(radarCfg);
    VisionSensor vision(visionCfg);
    LidarSensor lidar(lidarCfg);
    MagnetometerSensor magnetometer(magnetometerCfg);
    BarometerSensor baro(baroCfg);
    CelestialSensor celestial(celestialCfg);
    std::vector<SensorBase *> sensors{&gps, &thermal, &radar, &deadReckoning, &imu, &vision, &lidar, &magnetometer, &baro};
    if (celestialAllowed)
    {
        sensors.push_back(&celestial);
    }

    ModeManagerConfig modeConfig;
    modeConfig.permittedSensors = cfg.permittedSensors;
    modeConfig.celestialAllowed = celestialAllowed;
    modeConfig.celestialDatasetAvailable = celestialDatasetAvailable;
    modeConfig.maxDataAgeSeconds = cfg.fusion.maxDataAgeSeconds;
    modeConfig.minConfidence = cfg.fusion.minConfidence;
    modeConfig.minHealthyCount = cfg.mode.minHealthyCount;
    modeConfig.minDwellSteps = cfg.mode.minDwellSteps;
    modeConfig.maxStaleCount = cfg.mode.maxStaleCount;
    modeConfig.maxLowConfidenceCount = cfg.mode.maxLowConfidenceCount;
    modeConfig.lockoutSteps = cfg.mode.lockoutSteps;
    modeConfig.maxDisagreementCount = cfg.fusion.maxDisagreementCount;
    modeConfig.disagreementThreshold = cfg.fusion.disagreementThreshold;
    modeConfig.historyWindow = cfg.mode.historyWindow;
    modeConfig.maxResidualAgeSeconds = cfg.fusion.maxResidualAgeSeconds;
    modeConfig.ladderOrder = cfg.mode.ladderOrder;
    ModeManager modeManager(modeConfig);

    ModeScheduler scheduler(cfg.scheduler);

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
        Measurement visionMeas = vision.sample(state, dt, rng);
        Measurement lidarMeas = lidar.sample(state, dt, rng);
        Measurement magMeas = magnetometer.sample(state, dt, rng);
        Measurement baroMeas = baro.sample(state, dt, rng);
        Measurement celestialMeas;
        if (celestialAllowed)
        {
            celestialMeas = celestial.sample(state, dt, rng);
        }

        ModeDecisionDetail detail = modeManager.decideDetailed(sensors);
        std::vector<PipelineRequest> requests = {
            {"primary_scan", ModeType::Primary, true, false, cfg.scheduler.primaryBudgetMs, 0.0},
            {"ir_snapshot", ModeType::AuxSnapshot, true, true, cfg.scheduler.auxBudgetMs, 0.0},
            {"lidar_snapshot", ModeType::AuxSnapshot, true, true, cfg.scheduler.auxBudgetMs, 0.0}};
        ScheduleResult schedule = scheduler.schedule(requests, state.time);
        Projection2D xy = projectXY(state);
        Projection2D xz = projectXZ(state);

        std::cout << "Step " << i << " | model=" << static_cast<int>(model)
                  << " | mode=" << detail.selectedMode
                  << " | conf=" << detail.confidence
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
        if (visionMeas.valid && visionMeas.position)
        {
            std::cout << " | vision=(" << visionMeas.position->x << ", " << visionMeas.position->y << ", " << visionMeas.position->z << ")";
        }
        if (lidarMeas.valid && lidarMeas.range)
        {
            std::cout << " | lidar_range=" << *lidarMeas.range;
        }
        if (magMeas.valid && magMeas.heading)
        {
            std::cout << " | mag_heading_rad=" << *magMeas.heading;
        }
        if (baroMeas.valid && baroMeas.altitude)
        {
            std::cout << " | baro_alt=" << *baroMeas.altitude;
        }
        if (celestialAllowed && celestialMeas.valid && celestialMeas.position)
        {
            std::cout << " | celestial=(" << celestialMeas.position->x << ", " << celestialMeas.position->y << ", " << celestialMeas.position->z << ")";
        }
        if (!detail.reason.empty())
        {
            std::cout << " | decision=" << detail.reason;
        }
        if (!detail.contributors.empty())
        {
            std::cout << " | contributors=";
            for (size_t idx = 0; idx < detail.contributors.size(); ++idx)
            {
                std::cout << detail.contributors[idx];
                if (idx + 1 < detail.contributors.size())
                {
                    std::cout << ",";
                }
            }
        }
        if (!schedule.scheduled.empty())
        {
            std::cout << " | scheduled=";
            for (size_t idx = 0; idx < schedule.scheduled.size(); ++idx)
            {
                std::cout << schedule.scheduled[idx];
                if (idx + 1 < schedule.scheduled.size())
                {
                    std::cout << ",";
                }
            }
        }
        std::cout << "\n";
    }

    std::cout << "Simulation complete.\n";
    return 0;
}
