#include "core/mode_manager.h"
#include "core/sensors.h"
#include "core/simulation_utils.h"
#include "core/Tracker.h"
#include "core/HeatSignature.h"
#include "tools/sim_config_loader.h"
#include "tools/adapter_registry_loader.h"
#include "core/mode_scheduler.h"
#include "core/state.h"
#include "core/hash.h"

#include <cassert>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <limits>
#include <sstream>
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

    void setProvenance(ProvenanceTag tag)
    {
        provenance = tag;
    }

    void setHealthy(bool healthy)
    {
        if (healthy)
        {
            recordSuccess();
            status.hasMeasurement = true;
            status.lastMeasurement.valid = true;
        }
        else
        {
            recordFailure("forced");
            status.hasMeasurement = false;
            status.lastMeasurement.valid = false;
        }
        status.lastMeasurement.provenance = provenance;
    }

    void setStatus(bool healthy, double ageSeconds, double confidence)
    {
        status.healthy = healthy;
        status.timeSinceLastValid = ageSeconds;
        status.confidence = confidence;
        status.hasMeasurement = healthy;
        status.lastMeasurement.valid = healthy;
        status.lastMeasurement.provenance = provenance;
    }

    void setPositionMeasurement(const Vec3 &position)
    {
        Measurement measurement;
        measurement.position = position;
        measurement.valid = true;
        measurement.provenance = provenance;
        status.hasMeasurement = true;
        status.lastMeasurement = measurement;
    }

protected:
    Measurement generateMeasurement(const State9 &, std::mt19937 &) override
    {
        Measurement measurement;
        measurement.valid = true;
        return measurement;
    }

private:
    ProvenanceTag provenance = ProvenanceTag::Operational;
};

std::filesystem::path writeConfigFile(const std::string &name, const std::string &contents)
{
    std::filesystem::path path = std::filesystem::current_path() / name;
    std::ofstream file(path.string(), std::ios::trunc | std::ios::binary);
    file << contents;
    return path;
}

std::string todayDateIso()
{
    std::time_t now = std::time(nullptr);
    std::tm *utc = std::gmtime(&now);
    assert(utc != nullptr);
    std::ostringstream out;
    out << (utc->tm_year + 1900) << "-";
    if (utc->tm_mon + 1 < 10)
    {
        out << "0";
    }
    out << (utc->tm_mon + 1) << "-";
    if (utc->tm_mday < 10)
    {
        out << "0";
    }
    out << utc->tm_mday;
    return out.str();
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

    std::filesystem::path invalidLadder = writeConfigFile(
        "airtrace_invalid_ladder.cfg",
        "config.version=1.0\n"
        "mode.ladder_order=gps,invalid_mode,hold\n");
    ConfigResult invalidLadderResult = loadSimConfig(invalidLadder.string());
    assert(!invalidLadderResult.ok);
    std::filesystem::remove(invalidLadder);

    std::filesystem::path policyConfigPath = writeConfigFile(
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
    ConfigResult policyResult = loadSimConfig(policyConfigPath.string());
    assert(policyResult.ok);
    std::filesystem::remove(policyConfigPath);

    std::filesystem::path authMissingConfig = writeConfigFile(
        "airtrace_auth_missing.cfg",
        "config.version=1.0\n"
        "policy.authorization.version=1\n");
    ConfigResult authMissingResult = loadSimConfig(authMissingConfig.string());
    assert(!authMissingResult.ok);
    std::filesystem::remove(authMissingConfig);

    std::filesystem::path authInvalidModeConfig = writeConfigFile(
        "airtrace_auth_invalid_mode.cfg",
        "config.version=1.0\n"
        "policy.authorization.version=1\n"
        "policy.authorization.source=test\n"
        "policy.authorization.allowed_modes=gps,invalid_mode\n");
    ConfigResult authInvalidModeResult = loadSimConfig(authInvalidModeConfig.string());
    assert(!authInvalidModeResult.ok);
    std::filesystem::remove(authInvalidModeConfig);

    std::filesystem::path authValidConfig = writeConfigFile(
        "airtrace_auth_valid.cfg",
        "config.version=1.0\n"
        "policy.authorization.version=1\n"
        "policy.authorization.source=test\n"
        "policy.authorization.allowed_modes=gps,imu\n");
    ConfigResult authValidResult = loadSimConfig(authValidConfig.string());
    assert(authValidResult.ok);
    std::filesystem::remove(authValidConfig);

    std::filesystem::path provenanceConfig = writeConfigFile(
        "airtrace_provenance.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=operational\n"
        "provenance.allowed_inputs=operational\n"
        "provenance.allow_mixed=false\n"
        "provenance.unknown_action=deny\n");
    ConfigResult provenanceResult = loadSimConfig(provenanceConfig.string());
    assert(provenanceResult.ok);
    std::filesystem::remove(provenanceConfig);

    std::filesystem::path provenanceMixedConfig = writeConfigFile(
        "airtrace_provenance_mixed.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=operational\n"
        "provenance.allowed_inputs=operational,simulation\n"
        "provenance.allow_mixed=false\n");
    ConfigResult provenanceMixedResult = loadSimConfig(provenanceMixedConfig.string());
    assert(!provenanceMixedResult.ok);
    std::filesystem::remove(provenanceMixedConfig);

    std::filesystem::path provenanceInvalidConfig = writeConfigFile(
        "airtrace_provenance_invalid.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=invalid\n"
        "provenance.allowed_inputs=operational\n");
    ConfigResult provenanceInvalidResult = loadSimConfig(provenanceInvalidConfig.string());
    assert(!provenanceInvalidResult.ok);
    std::filesystem::remove(provenanceInvalidConfig);

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

    std::filesystem::path caseInsensitiveRoleConfig = writeConfigFile(
        "airtrace_case_insensitive_role.cfg",
        "config.version=1.0\n"
        "policy.roles=operator,supervisor\n"
        "policy.active_role=Operator\n"
        "policy.role_permissions.SuperVisor=network_aid\n");
    ConfigResult caseInsensitiveRoleResult = loadSimConfig(caseInsensitiveRoleConfig.string());
    assert(caseInsensitiveRoleResult.ok);
    std::filesystem::remove(caseInsensitiveRoleConfig);

    std::filesystem::path missingHashConfig = writeConfigFile(
        "airtrace_missing_hash.cfg",
        "config.version=1.0\n"
        "dataset.celestial.catalog_path=data/catalog\n");
    ConfigResult missingHashResult = loadSimConfig(missingHashConfig.string());
    assert(!missingHashResult.ok);
    std::filesystem::remove(missingHashConfig);

    std::filesystem::path invalidSurfaceConfig = writeConfigFile(
        "airtrace_invalid_surface.cfg",
        "config.version=1.0\n"
        "ui.surface=invalid\n");
    ConfigResult invalidSurfaceResult = loadSimConfig(invalidSurfaceConfig.string());
    assert(!invalidSurfaceResult.ok);
    std::filesystem::remove(invalidSurfaceConfig);

    std::filesystem::path missingAdapterVersionConfig = writeConfigFile(
        "airtrace_missing_adapter_version.cfg",
        "config.version=1.0\n"
        "adapter.id=air\n");
    ConfigResult missingAdapterVersionResult = loadSimConfig(missingAdapterVersionConfig.string());
    assert(!missingAdapterVersionResult.ok);
    std::filesystem::remove(missingAdapterVersionConfig);

    std::filesystem::path unknownAdapterConfig = writeConfigFile(
        "airtrace_unknown_adapter.cfg",
        "config.version=1.0\n"
        "adapter.id=custom\n"
        "adapter.version=1.0.0\n");
    ConfigResult unknownAdapterResult = loadSimConfig(unknownAdapterConfig.string());
    assert(!unknownAdapterResult.ok);
    std::filesystem::remove(unknownAdapterConfig);

    std::filesystem::path invalidAdapterVersionContextConfig = writeConfigFile(
        "airtrace_invalid_adapter_context.cfg",
        "config.version=1.0\n"
        "adapter.core_version=1.0\n");
    ConfigResult invalidAdapterVersionContextResult = loadSimConfig(invalidAdapterVersionContextConfig.string());
    assert(!invalidAdapterVersionContextResult.ok);
    std::filesystem::remove(invalidAdapterVersionContextConfig);

    std::filesystem::path invalidAllowlistAgeConfig = writeConfigFile(
        "airtrace_invalid_allowlist_age.cfg",
        "config.version=1.0\n"
        "adapter.allowlist_max_age_days=-1\n");
    ConfigResult invalidAllowlistAgeResult = loadSimConfig(invalidAllowlistAgeConfig.string());
    assert(!invalidAllowlistAgeResult.ok);
    std::filesystem::remove(invalidAllowlistAgeConfig);

    std::filesystem::path unauthorizedPluginConfig = writeConfigFile(
        "airtrace_plugin_unauthorized.cfg",
        "config.version=1.0\n"
        "plugin.id=nav_plugin\n"
        "plugin.version=1.0.0\n"
        "plugin.signature_hash=1111111111111111111111111111111111111111111111111111111111111111\n"
        "plugin.signature_algorithm=sha256\n"
        "plugin.allowlist.id=nav_plugin\n"
        "plugin.allowlist.version=1.0.0\n"
        "plugin.allowlist.signature_hash=1111111111111111111111111111111111111111111111111111111111111111\n"
        "plugin.allowlist.signature_algorithm=sha256\n"
        "plugin.authorization_required=true\n"
        "plugin.authorization_granted=false\n");
    ConfigResult unauthorizedPluginResult = loadSimConfig(unauthorizedPluginConfig.string());
    assert(!unauthorizedPluginResult.ok);
    bool sawUnauthorizedPlugin = false;
    for (const auto &issue : unauthorizedPluginResult.issues)
    {
        if (issue.message.find("plugin_not_authorized") != std::string::npos)
        {
            sawUnauthorizedPlugin = true;
            break;
        }
    }
    assert(sawUnauthorizedPlugin);
    std::filesystem::remove(unauthorizedPluginConfig);

    std::filesystem::path unsignedPluginConfig = writeConfigFile(
        "airtrace_plugin_unsigned.cfg",
        "config.version=1.0\n"
        "plugin.id=nav_plugin\n"
        "plugin.version=1.0.0\n"
        "plugin.signature_hash=1111111111111111111111111111111111111111111111111111111111111111\n"
        "plugin.signature_algorithm=sha256\n"
        "plugin.allowlist.id=nav_plugin\n"
        "plugin.allowlist.version=1.0.0\n"
        "plugin.allowlist.signature_hash=2222222222222222222222222222222222222222222222222222222222222222\n"
        "plugin.allowlist.signature_algorithm=sha256\n"
        "plugin.authorization_required=true\n"
        "plugin.authorization_granted=true\n");
    ConfigResult unsignedPluginResult = loadSimConfig(unsignedPluginConfig.string());
    assert(!unsignedPluginResult.ok);
    bool sawInvalidSignature = false;
    for (const auto &issue : unsignedPluginResult.issues)
    {
        if (issue.message.find("plugin_signature_invalid") != std::string::npos)
        {
            sawInvalidSignature = true;
            break;
        }
    }
    assert(sawInvalidSignature);
    std::filesystem::remove(unsignedPluginConfig);

    std::filesystem::path validPluginConfig = writeConfigFile(
        "airtrace_plugin_valid.cfg",
        "config.version=1.0\n"
        "plugin.id=nav_plugin\n"
        "plugin.version=1.0.0\n"
        "plugin.signature_hash=1111111111111111111111111111111111111111111111111111111111111111\n"
        "plugin.signature_algorithm=sha256\n"
        "plugin.allowlist.id=nav_plugin\n"
        "plugin.allowlist.version=1.0.0\n"
        "plugin.allowlist.signature_hash=1111111111111111111111111111111111111111111111111111111111111111\n"
        "plugin.allowlist.signature_algorithm=sha256\n"
        "plugin.authorization_required=true\n"
        "plugin.authorization_granted=true\n");
    ConfigResult validPluginResult = loadSimConfig(validPluginConfig.string());
    assert(validPluginResult.ok);
    std::filesystem::remove(validPluginConfig);

    const std::string manifestContent =
        "{\n"
        "  \"adapter.id\": \"custom_adapter\",\n"
        "  \"adapter.version\": \"1.0.0\",\n"
        "  \"adapter.contract_version\": \"1.0.0\",\n"
        "  \"ui.contract_version\": \"1.0.0\",\n"
        "  \"core.compatibility.min\": \"1.0.0\",\n"
        "  \"core.compatibility.max\": \"1.0.0\",\n"
        "  \"tools.compatibility.min\": \"1.0.0\",\n"
        "  \"tools.compatibility.max\": \"1.0.0\",\n"
        "  \"ui.compatibility.min\": \"1.0.0\",\n"
        "  \"ui.compatibility.max\": \"1.0.0\",\n"
        "  \"capabilities\": [\n"
        "    {\n"
        "      \"id\": \"gps_fix\",\n"
        "      \"description\": \"GPS fix\",\n"
        "      \"units\": \"meters\",\n"
        "      \"range_min\": 0,\n"
        "      \"range_max\": 100000,\n"
        "      \"error_behavior\": \"deny\"\n"
        "    }\n"
        "  ],\n"
        "  \"ui_extensions\": [\n"
        "    {\n"
        "      \"field_id\": \"gps_hdop\",\n"
        "      \"type\": \"number\",\n"
        "      \"units\": \"ratio\",\n"
        "      \"range_min\": 0,\n"
        "      \"range_max\": 50,\n"
        "      \"error_behavior\": \"hide\",\n"
        "      \"surfaces\": [\"tui\"]\n"
        "    }\n"
        "  ]\n"
        "}\n";
    const std::filesystem::path manifestPath = writeConfigFile("airtrace_adapter_manifest.json", manifestContent);
    std::vector<unsigned char> manifestBytes(manifestContent.begin(), manifestContent.end());
    const std::string manifestHash = sha256Hex(manifestBytes);

    std::ostringstream staleAllowlist;
    staleAllowlist
        << "{\n"
        << "  \"entries\": [\n"
        << "    {\n"
        << "      \"adapter.id\": \"custom_adapter\",\n"
        << "      \"adapter.version\": \"1.0.0\",\n"
        << "      \"signature.hash\": \"" << manifestHash << "\",\n"
        << "      \"signature.algorithm\": \"sha256\",\n"
        << "      \"approved_by\": \"qa\",\n"
        << "      \"approval_date\": \"2000-01-01\",\n"
        << "      \"allowed_surfaces\": [\"tui\"]\n"
        << "    }\n"
        << "  ]\n"
        << "}\n";
    const std::filesystem::path staleAllowlistPath = writeConfigFile("airtrace_adapter_allowlist_stale.json", staleAllowlist.str());

    std::ostringstream staleAdapterConfig;
    staleAdapterConfig
        << "config.version=1.0\n"
        << "adapter.id=custom_adapter\n"
        << "adapter.version=1.0.0\n"
        << "adapter.manifest_path=" << manifestPath.string() << "\n"
        << "adapter.allowlist_path=" << staleAllowlistPath.string() << "\n"
        << "adapter.allowlist_max_age_days=30\n"
        << "ui.surface=tui\n";
    const std::filesystem::path staleAdapterConfigPath = writeConfigFile("airtrace_adapter_stale.cfg", staleAdapterConfig.str());
    ConfigResult staleAdapterConfigResult = loadSimConfig(staleAdapterConfigPath.string());
    assert(!staleAdapterConfigResult.ok);
    bool sawStaleReason = false;
    for (const auto &issue : staleAdapterConfigResult.issues)
    {
        if (issue.message.find("adapter_allowlist_stale") != std::string::npos)
        {
            sawStaleReason = true;
            break;
        }
    }
    assert(sawStaleReason);
    std::filesystem::remove(staleAdapterConfigPath);
    std::filesystem::remove(staleAllowlistPath);

    std::ostringstream freshAllowlist;
    freshAllowlist
        << "{\n"
        << "  \"entries\": [\n"
        << "    {\n"
        << "      \"adapter.id\": \"custom_adapter\",\n"
        << "      \"adapter.version\": \"1.0.0\",\n"
        << "      \"signature.hash\": \"" << manifestHash << "\",\n"
        << "      \"signature.algorithm\": \"sha256\",\n"
        << "      \"approved_by\": \"qa\",\n"
        << "      \"approval_date\": \"" << todayDateIso() << "\",\n"
        << "      \"allowed_surfaces\": [\"tui\"]\n"
        << "    }\n"
        << "  ]\n"
        << "}\n";
    const std::filesystem::path freshAllowlistPath = writeConfigFile("airtrace_adapter_allowlist_fresh.json", freshAllowlist.str());

    SimConfig freshAdapterConfig;
    freshAdapterConfig.adapter.id = "custom_adapter";
    freshAdapterConfig.adapter.version = "1.0.0";
    freshAdapterConfig.adapter.manifestPath = manifestPath.string();
    freshAdapterConfig.adapter.allowlistPath = freshAllowlistPath.string();
    freshAdapterConfig.adapter.allowlistMaxAgeDays = 30;
    freshAdapterConfig.adapter.uiSurface = "tui";
    freshAdapterConfig.adapter.coreVersion = "1.0.0";
    freshAdapterConfig.adapter.toolsVersion = "1.0.0";
    freshAdapterConfig.adapter.uiVersion = "1.0.0";
    freshAdapterConfig.adapter.adapterContractVersion = "1.0.0";
    freshAdapterConfig.adapter.uiContractVersion = "1.0.0";
    tools::AdapterUiSnapshot adapterSnapshot = tools::loadAdapterUiSnapshot(freshAdapterConfig);
    assert(adapterSnapshot.status == "ok");
    assert(adapterSnapshot.reason == "ok");
    assert(adapterSnapshot.approvedBy == "qa");
    assert(adapterSnapshot.contextVersionSummary.find("core=1.0.0") != std::string::npos);
    std::filesystem::remove(freshAllowlistPath);
    std::filesystem::remove(manifestPath);

    std::vector<unsigned char> abc = {'a', 'b', 'c'};
    std::string abcHash = sha256Hex(abc);
    assert(hashEquals("BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD", abcHash));

    std::vector<Object> seededTargetsA = generateTargets(5, 100, 1337);
    std::vector<Object> seededTargetsB = generateTargets(5, 100, 1337);
    assert(seededTargetsA.size() == seededTargetsB.size());
    for (size_t idx = 0; idx < seededTargetsA.size(); ++idx)
    {
        assert(seededTargetsA[idx].getPosition() == seededTargetsB[idx].getPosition());
    }
    assert(generateTargets(5, 100, 1337)[0].getPosition() != generateTargets(5, 100, 1338)[0].getPosition());

    Object follower(1, "follower", std::pair<int, int>{0, 0});
    Object target(2, "target", std::pair<int, int>{10, 10});
    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    tracker.setTarget(target);
    tracker.startTracking(1, 0);
    assert(!tracker.isTrackingActive());

    HeatSignature heatSignature;
    heatSignature.setHeatData(10.0f);
    const std::pair<int, int> lowHeatStep = heatSignature.calculatePath(follower, target);
    heatSignature.setHeatData(80.0f);
    const std::pair<int, int> highHeatStep = heatSignature.calculatePath(follower, target);
    assert(highHeatStep.first > lowHeatStep.first);

    heatSignature.setHeatData(-5.0f);
    const std::pair<int, int> clampedLowStep = heatSignature.calculatePath(follower, target);
    assert(clampedLowStep.first == 0);
    assert(clampedLowStep.second == 0);

    heatSignature.setHeatData(200.0f);
    const std::pair<int, int> clampedHighStep = heatSignature.calculatePath(follower, target);
    assert(clampedHighStep.first == 5);
    assert(clampedHighStep.second == 5);

    heatSignature.setHeatData(std::numeric_limits<float>::quiet_NaN());
    const std::pair<int, int> nanStep = heatSignature.calculatePath(follower, target);
    assert(nanStep.first == 0);
    assert(nanStep.second == 0);

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
    ModeDecisionDetail detail = manager.getLastDecisionDetail();
    assert(detail.selectedMode == "gps");
    assert(detail.contributors.size() == 1);
    assert(detail.contributors[0] == "gps");

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

    ModeManagerConfig authConfig;
    authConfig.minHealthyCount = 1;
    authConfig.authorizationRequired = true;
    authConfig.authorizationVerified = false;
    authConfig.authorizationAllowedModes = {"gps"};
    authConfig.permittedSensors = {"gps"};
    authConfig.ladderOrder = {"gps", "hold"};
    ModeManager authManager(authConfig);
    gps.setHealthy(true);
    decision = authManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);
    assert(decision.reason == "auth_unavailable");

    authConfig.authorizationVerified = true;
    ModeManager authVerifiedManager(authConfig);
    decision = authVerifiedManager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    authConfig.authorizationAllowedModes = {"thermal"};
    ModeManager authDeniedManager(authConfig);
    decision = authDeniedManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);
    assert(decision.reason == "auth_denied");

    ModeManagerConfig provenanceModeConfig;
    provenanceModeConfig.minHealthyCount = 1;
    provenanceModeConfig.permittedSensors = {"gps"};
    provenanceModeConfig.ladderOrder = {"gps", "hold"};
    provenanceModeConfig.allowedProvenances = {ProvenanceTag::Simulation};
    ModeManager provenanceManager(provenanceModeConfig);
    gps.setProvenance(ProvenanceTag::Operational);
    gps.setHealthy(true);
    decision = provenanceManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);
    assert(provenanceManager.getLastDecisionDetail().downgradeReason == "provenance_denied");

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

    ModeManagerConfig gatingConfig;
    gatingConfig.minHealthyCount = 1;
    gatingConfig.maxDataAgeSeconds = 0.5;
    gatingConfig.minConfidence = 0.6;
    gatingConfig.maxStaleCount = 2;
    gatingConfig.maxLowConfidenceCount = 2;
    gatingConfig.lockoutSteps = 2;
    gatingConfig.permittedSensors = {"gps"};
    ModeManager gatingManager(gatingConfig);
    TestSensor gatingGps("gps");
    std::vector<SensorBase *> gatingSensors{&gatingGps};

    gatingGps.setStatus(true, 1.0, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Hold);

    gatingGps.setStatus(true, 0.1, 0.2);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Hold);

    gatingGps.setStatus(true, 0.1, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Gps);

    gatingGps.setStatus(true, 1.0, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Hold);

    gatingGps.setStatus(true, 1.0, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Hold);

    gatingGps.setStatus(true, 0.1, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Hold);

    gatingGps.setStatus(true, 0.1, 0.9);
    decision = gatingManager.decide(gatingSensors);
    assert(decision.mode == TrackingMode::Gps);

    ModeManagerConfig datasetConfig;
    datasetConfig.minHealthyCount = 1;
    datasetConfig.celestialAllowed = true;
    datasetConfig.celestialDatasetAvailable = false;
    datasetConfig.permittedSensors = {"celestial"};
    ModeManager datasetManager(datasetConfig);
    TestSensor datasetCelestial("celestial");
    std::vector<SensorBase *> datasetSensors{&datasetCelestial};
    datasetCelestial.setHealthy(true);
    decision = datasetManager.decide(datasetSensors);
    assert(decision.mode == TrackingMode::Hold);

    ModeManagerConfig residualConfig;
    residualConfig.minHealthyCount = 1;
    residualConfig.disagreementThreshold = 5.0;
    residualConfig.maxDisagreementCount = 1;
    residualConfig.lockoutSteps = 2;
    residualConfig.permittedSensors = {"gps", "imu"};
    residualConfig.ladderOrder = {"gps_ins"};
    ModeManager residualManager(residualConfig);
    TestSensor residualGps("gps");
    TestSensor residualImu("imu");
    std::vector<SensorBase *> residualSensors{&residualGps, &residualImu};
    residualGps.setHealthy(true);
    residualImu.setHealthy(true);
    residualGps.setPositionMeasurement({0.0, 0.0, 0.0});
    residualImu.setPositionMeasurement({100.0, 0.0, 0.0});
    decision = residualManager.decide(residualSensors);
    assert(decision.mode == TrackingMode::Hold);

    residualGps.setPositionMeasurement({0.0, 0.0, 0.0});
    residualImu.setPositionMeasurement({0.0, 0.0, 0.0});
    decision = residualManager.decide(residualSensors);
    assert(decision.mode == TrackingMode::Hold);

    residualGps.setPositionMeasurement({0.0, 0.0, 0.0});
    residualImu.setPositionMeasurement({0.0, 0.0, 0.0});
    decision = residualManager.decide(residualSensors);
    assert(decision.mode == TrackingMode::GpsIns);

    ModeManagerConfig historyConfig;
    historyConfig.minHealthyCount = 1;
    historyConfig.minConfidence = 0.5;
    historyConfig.maxLowConfidenceCount = 2;
    historyConfig.lockoutSteps = 2;
    historyConfig.historyWindow = 3;
    historyConfig.permittedSensors = {"gps"};
    historyConfig.ladderOrder = {"gps"};
    ModeManager historyManager(historyConfig);
    TestSensor historyGps("gps");
    std::vector<SensorBase *> historySensors{&historyGps};

    historyGps.setStatus(true, 0.1, 0.2);
    decision = historyManager.decide(historySensors);
    assert(decision.mode == TrackingMode::Hold);

    historyGps.setStatus(true, 0.1, 0.2);
    decision = historyManager.decide(historySensors);
    assert(decision.mode == TrackingMode::Hold);

    historyGps.setStatus(true, 0.1, 0.9);
    decision = historyManager.decide(historySensors);
    assert(decision.mode == TrackingMode::Hold);

    historyGps.setStatus(true, 0.1, 0.9);
    decision = historyManager.decide(historySensors);
    assert(decision.mode == TrackingMode::Hold);

    historyGps.setStatus(true, 0.1, 0.9);
    decision = historyManager.decide(historySensors);
    assert(decision.mode == TrackingMode::Gps);

    ModeManagerConfig jitterConfig;
    jitterConfig.minHealthyCount = 1;
    jitterConfig.maxDataAgeSeconds = 0.2;
    jitterConfig.maxStaleCount = 2;
    jitterConfig.lockoutSteps = 2;
    jitterConfig.historyWindow = 3;
    jitterConfig.permittedSensors = {"gps"};
    jitterConfig.ladderOrder = {"gps"};
    ModeManager jitterManager(jitterConfig);
    TestSensor jitterGps("gps");
    std::vector<SensorBase *> jitterSensors{&jitterGps};

    jitterGps.setStatus(true, 0.3, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.25, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Gps);

    ModeManagerConfig multipathConfig;
    multipathConfig.minHealthyCount = 1;
    multipathConfig.disagreementThreshold = 5.0;
    multipathConfig.maxDisagreementCount = 1;
    multipathConfig.lockoutSteps = 2;
    multipathConfig.permittedSensors = {"lidar", "imu"};
    multipathConfig.ladderOrder = {"lio"};
    ModeManager multipathManager(multipathConfig);
    TestSensor multipathLidar("lidar");
    TestSensor multipathImu("imu");
    std::vector<SensorBase *> multipathSensors{&multipathLidar, &multipathImu};
    multipathLidar.setHealthy(true);
    multipathImu.setHealthy(true);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0});
    multipathImu.setPositionMeasurement({100.0, 0.0, 0.0});
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Hold);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0});
    multipathImu.setPositionMeasurement({0.0, 0.0, 0.0});
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Hold);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0});
    multipathImu.setPositionMeasurement({0.0, 0.0, 0.0});
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Lio);

    ModeManagerConfig saturationConfig;
    saturationConfig.minHealthyCount = 1;
    saturationConfig.minConfidence = 0.5;
    saturationConfig.permittedSensors = {"thermal", "radar"};
    saturationConfig.ladderOrder = {"thermal", "radar"};
    ModeManager saturationManager(saturationConfig);
    TestSensor saturationThermal("thermal");
    TestSensor saturationRadar("radar");
    std::vector<SensorBase *> saturationSensors{&saturationThermal, &saturationRadar};
    saturationThermal.setStatus(true, 0.1, 0.1);
    saturationRadar.setStatus(true, 0.1, 0.9);
    decision = saturationManager.decide(saturationSensors);
    assert(decision.mode == TrackingMode::Radar);

    SchedulerConfig schedulerConfig;
    schedulerConfig.primaryBudgetMs = 5.0;
    schedulerConfig.auxBudgetMs = 2.0;
    schedulerConfig.maxAuxPipelines = 1;
    schedulerConfig.auxMinServiceIntervalSeconds = 1.0;
    schedulerConfig.allowSnapshotOverlap = true;

    ModeScheduler scheduler(schedulerConfig);
    std::vector<PipelineRequest> requests = {
        {"primary_scan", ModeType::Primary, true, false, 1.0, 0.0},
        {"ir_snapshot", ModeType::AuxSnapshot, true, true, 1.5, 0.0},
        {"lidar_snapshot", ModeType::AuxSnapshot, true, true, 1.5, 0.0}};

    ScheduleResult scheduled = scheduler.schedule(requests, 10.0);
    assert(scheduled.scheduled.size() == 2);
    assert(scheduled.scheduled[0] == "primary_scan");
    assert(scheduled.scheduled[1] == "ir_snapshot");

    schedulerConfig.allowSnapshotOverlap = false;
    ModeScheduler noOverlapScheduler(schedulerConfig);
    ScheduleResult noOverlap = noOverlapScheduler.schedule(requests, 10.0);
    assert(noOverlap.scheduled.size() == 1);
    assert(noOverlap.scheduled[0] == "primary_scan");

    return 0;
}
