#include "core/mode_manager.h"
#include "core/sensors.h"
#include "core/sim_config.h"
#include "core/state.h"
#include "core/hash.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
class TestSensor : public SensorBase
{
public:
    explicit TestSensor(std::string name)
        : SensorBase(std::move(name), {1.0, 0.0, 0.0, 0.0, 1.0})
    {
    }

    void setHealthy(bool healthy)
    {
        if (healthy)
        {
            recordSuccess();
        }
        else
        {
            recordFailure("forced");
        }
    }

protected:
    Measurement generateMeasurement(const State9 &, std::mt19937 &) override
    {
        Measurement measurement;
        measurement.valid = true;
        return measurement;
    }
};

std::filesystem::path writeConfigFile(const std::string &name, const std::string &contents)
{
    std::filesystem::path path = std::filesystem::current_path() / name;
    std::ofstream file(path.string(), std::ios::trunc);
    file << contents;
    return path;
}
} // namespace

int main()
{
    State9 state{};
    state.position = {0.0, 0.0, 0.0};
    state.velocity = {10.0, -5.0, 2.0};
    state.acceleration = {1.0, 0.0, -1.0};
    state.time = 0.0;

    const double dt = 2.0;
    State9 next = integrateState(state, dt);

    const double eps = 1e-9;
    assert(std::fabs(next.position.x - 22.0) < eps);
    assert(std::fabs(next.position.y - -10.0) < eps);
    assert(std::fabs(next.position.z - 2.0) < eps);
    assert(std::fabs(next.velocity.x - 12.0) < eps);
    assert(std::fabs(next.velocity.y - -5.0) < eps);
    assert(std::fabs(next.velocity.z - 0.0) < eps);
    assert(std::fabs(next.time - 2.0) < eps);

    Projection2D xy = projectXY(next);
    assert(xy.plane == "XY");
    assert(std::fabs(xy.x - 22.0) < eps);
    assert(std::fabs(xy.y - -10.0) < eps);

    std::filesystem::path missingVersion = writeConfigFile(
        "airtrace_missing_version.cfg",
        "sim.dt=0.2\n");
    ConfigResult missingResult = loadSimConfig(missingVersion.string());
    assert(!missingResult.ok);
    std::filesystem::remove(missingVersion);

    std::filesystem::path invalidDt = writeConfigFile(
        "airtrace_invalid_dt.cfg",
        "config.version=1.0\n"
        "sim.dt=0\n"
        "bounds.min.x=10\n"
        "bounds.max.x=0\n");
    ConfigResult invalidResult = loadSimConfig(invalidDt.string());
    assert(!invalidResult.ok);
    std::filesystem::remove(invalidDt);

    std::filesystem::path policyConfig = writeConfigFile(
        "airtrace_policy.cfg",
        "config.version=1.0\n"
        "platform.profile=handheld\n"
        "platform.permitted_sensors=gps,imu,celestial\n"
        "policy.network_aid.mode=deny\n"
        "policy.network_aid.override_required=true\n"
        "policy.network_aid.override_auth=credential\n"
        "policy.network_aid.override_timeout_seconds=60\n"
        "policy.roles=operator,supervisor\n"
        "policy.active_role=operator\n"
        "policy.role_permissions.operator=network_aid\n"
        "dataset.celestial.tier=minimal\n"
        "dataset.celestial.catalog_path=data/catalog\n"
        "dataset.celestial.catalog_hash=abc\n"
        "dataset.celestial.ephemeris_path=data/eph\n"
        "dataset.celestial.ephemeris_hash=def\n");
    ConfigResult policyResult = loadSimConfig(policyConfig.string());
    assert(policyResult.ok);
    std::filesystem::remove(policyConfig);

    std::filesystem::path sensorConfig = writeConfigFile(
        "airtrace_sensor_config.cfg",
        "config.version=1.0\n"
        "sensor.vision.rate_hz=10\n"
        "sensor.vision.noise_std=1\n"
        "sensor.vision.dropout=0\n"
        "sensor.vision.false_positive=0\n"
        "sensor.vision.max_range=2000\n"
        "sensor.lidar.rate_hz=5\n"
        "sensor.lidar.noise_std=0.5\n"
        "sensor.lidar.dropout=0\n"
        "sensor.lidar.false_positive=0\n"
        "sensor.lidar.max_range=1500\n"
        "sensor.magnetometer.rate_hz=20\n"
        "sensor.magnetometer.noise_std=0.05\n"
        "sensor.magnetometer.dropout=0\n"
        "sensor.magnetometer.false_positive=0\n"
        "sensor.magnetometer.max_range=1000000\n"
        "sensor.baro.rate_hz=2\n"
        "sensor.baro.noise_std=0.3\n"
        "sensor.baro.dropout=0\n"
        "sensor.baro.false_positive=0\n"
        "sensor.baro.max_range=20000\n"
        "sensor.celestial.rate_hz=1\n"
        "sensor.celestial.noise_std=0.5\n"
        "sensor.celestial.dropout=0\n"
        "sensor.celestial.false_positive=0\n"
        "sensor.celestial.max_range=10000000\n");
    ConfigResult sensorResult = loadSimConfig(sensorConfig.string());
    assert(sensorResult.ok);
    std::filesystem::remove(sensorConfig);

    std::filesystem::path badRoleConfig = writeConfigFile(
        "airtrace_bad_role.cfg",
        "config.version=1.0\n"
        "policy.roles=operator\n"
        "policy.role_permissions.supervisor=network_aid\n");
    ConfigResult badRoleResult = loadSimConfig(badRoleConfig.string());
    assert(!badRoleResult.ok);
    std::filesystem::remove(badRoleConfig);

    std::filesystem::path badActiveRoleConfig = writeConfigFile(
        "airtrace_bad_active_role.cfg",
        "config.version=1.0\n"
        "policy.roles=operator\n"
        "policy.active_role=supervisor\n");
    ConfigResult badActiveRoleResult = loadSimConfig(badActiveRoleConfig.string());
    assert(!badActiveRoleResult.ok);
    std::filesystem::remove(badActiveRoleConfig);

    std::filesystem::path missingHashConfig = writeConfigFile(
        "airtrace_missing_hash.cfg",
        "config.version=1.0\n"
        "dataset.celestial.catalog_path=data/catalog\n");
    ConfigResult missingHashResult = loadSimConfig(missingHashConfig.string());
    assert(!missingHashResult.ok);
    std::filesystem::remove(missingHashConfig);

    std::vector<unsigned char> abc = {'a', 'b', 'c'};
    std::string abcHash = sha256Hex(abc);
    assert(hashEquals("BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD", abcHash));

    std::mt19937 rng(42);
    SensorConfig testConfig{1.0, 0.0, 0.0, 0.0, 5000.0};
    VisionSensor vision(testConfig);
    LidarSensor lidar(testConfig);
    MagnetometerSensor magnetometer(testConfig);
    BarometerSensor baro(testConfig);
    CelestialSensor celestialSensor(testConfig);

    Measurement visionMeas = vision.sample(state, 1.0, rng);
    assert(visionMeas.valid);
    assert(visionMeas.position.has_value());

    Measurement lidarMeas = lidar.sample(state, 1.0, rng);
    assert(lidarMeas.valid);
    assert(lidarMeas.range.has_value());
    assert(lidarMeas.bearing.has_value());

    Measurement magMeas = magnetometer.sample(state, 1.0, rng);
    assert(magMeas.valid);
    assert(magMeas.heading.has_value());

    Measurement baroMeas = baro.sample(state, 1.0, rng);
    assert(baroMeas.valid);
    assert(baroMeas.altitude.has_value());

    Measurement celestialMeas = celestialSensor.sample(state, 1.0, rng);
    assert(celestialMeas.valid);
    assert(celestialMeas.position.has_value());

    ModeManagerConfig modeConfig;
    modeConfig.minHealthyCount = 2;
    modeConfig.minDwellSteps = 2;

    ModeManager manager(modeConfig);
    TestSensor gps("gps");
    TestSensor thermal("thermal");
    std::vector<SensorBase *> sensors{&gps, &thermal};

    gps.setHealthy(true);
    thermal.setHealthy(false);
    ModeDecision decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);

    gps.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    thermal.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    gps.setHealthy(false);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    gps.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    ModeManagerConfig policyConfig;
    policyConfig.minHealthyCount = 1;
    policyConfig.permittedSensors = {"gps"};
    policyConfig.celestialAllowed = false;
    ModeManager policyManager(policyConfig);
    gps.setHealthy(true);
    thermal.setHealthy(true);
    decision = policyManager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    gps.setHealthy(false);
    decision = policyManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);

    TestSensor celestial("celestial");
    std::vector<SensorBase *> celestialSensors{&celestial};
    policyConfig.permittedSensors = {"celestial"};
    policyConfig.celestialAllowed = false;
    ModeManager celestialManager(policyConfig);
    celestial.setHealthy(true);
    decision = celestialManager.decide(celestialSensors);
    assert(decision.mode == TrackingMode::Hold);

    policyConfig.celestialAllowed = true;
    ModeManager celestialAllowedManager(policyConfig);
    decision = celestialAllowedManager.decide(celestialSensors);
    assert(decision.mode == TrackingMode::Celestial);

    ModeManagerConfig ladderConfig;
    ladderConfig.minHealthyCount = 1;
    ladderConfig.minDwellSteps = 0;
    ModeManager ladderManager(ladderConfig);
    TestSensor visionSensor("vision");
    TestSensor lidarSensor("lidar");
    std::vector<SensorBase *> ladderSensors{&gps, &visionSensor, &lidarSensor};

    gps.setHealthy(false);
    visionSensor.setHealthy(true);
    lidarSensor.setHealthy(true);
    decision = ladderManager.decide(ladderSensors);
    assert(decision.mode == TrackingMode::Vision);

    visionSensor.setHealthy(false);
    decision = ladderManager.decide(ladderSensors);
    assert(decision.mode == TrackingMode::Lidar);

    return 0;
}
