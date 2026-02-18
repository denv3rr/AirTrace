#include "core/mode_manager.h"
#include "core/sensors.h"
#include "core/simulation_utils.h"
#include "core/Tracker.h"
#include "core/HeatSignature.h"
#include "tools/sim_config_loader.h"
#include "tools/adapter_registry_loader.h"
#include "tools/io_packager.h"
#include "tools/federation_bridge.h"
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

    std::filesystem::path rolePresetConfig = writeConfigFile(
        "airtrace_role_preset_valid.cfg",
        "config.version=1.0\n"
        "policy.roles=pilot,c2\n"
        "policy.active_role=pilot\n"
        "policy.role_permissions.pilot=front_view_workbench\n"
        "policy.role_permissions.c2=platform_workbench\n"
        "policy.role_preset.pilot.ui_surface=cockpit\n"
        "policy.role_preset.pilot.front_view_enabled=true\n"
        "policy.role_preset.pilot.front_view_families=eo_gray,ir_white_hot\n"
        "policy.role_preset.c2.ui_surface=c2\n"
        "policy.role_preset.c2.front_view_enabled=false\n"
        "policy.role_preset.c2.front_view_families=proximity_2d\n");
    ConfigResult rolePresetResult = loadSimConfig(rolePresetConfig.string());
    assert(rolePresetResult.ok);
    assert(rolePresetResult.config.policy.roleUiPresets.count("pilot") == 1U);
    const auto &pilotPreset = rolePresetResult.config.policy.roleUiPresets.at("pilot");
    assert(pilotPreset.uiSurface == "cockpit");
    assert(pilotPreset.hasFrontViewEnabled);
    assert(pilotPreset.frontViewEnabled);
    assert(pilotPreset.hasFrontViewFamilies);
    assert(pilotPreset.frontViewFamilies.size() == 2U);
    std::filesystem::remove(rolePresetConfig);

    std::filesystem::path rolePresetUndefinedRoleConfig = writeConfigFile(
        "airtrace_role_preset_undefined_role.cfg",
        "config.version=1.0\n"
        "policy.roles=operator\n"
        "policy.active_role=operator\n"
        "policy.role_preset.pilot.ui_surface=cockpit\n");
    ConfigResult rolePresetUndefinedRoleResult = loadSimConfig(rolePresetUndefinedRoleConfig.string());
    assert(!rolePresetUndefinedRoleResult.ok);
    std::filesystem::remove(rolePresetUndefinedRoleConfig);

    std::filesystem::path rolePresetMissingActiveConfig = writeConfigFile(
        "airtrace_role_preset_missing_active.cfg",
        "config.version=1.0\n"
        "policy.roles=operator,supervisor\n"
        "policy.active_role=operator\n"
        "policy.role_preset.supervisor.ui_surface=c2\n");
    ConfigResult rolePresetMissingActiveResult = loadSimConfig(rolePresetMissingActiveConfig.string());
    assert(!rolePresetMissingActiveResult.ok);
    std::filesystem::remove(rolePresetMissingActiveConfig);

    std::filesystem::path rolePresetInvalidFamilyConfig = writeConfigFile(
        "airtrace_role_preset_invalid_family.cfg",
        "config.version=1.0\n"
        "policy.roles=operator\n"
        "policy.active_role=operator\n"
        "policy.role_preset.operator.front_view_families=eo_gray,invalid_mode\n");
    ConfigResult rolePresetInvalidFamilyResult = loadSimConfig(rolePresetInvalidFamilyConfig.string());
    assert(!rolePresetInvalidFamilyResult.ok);
    std::filesystem::remove(rolePresetInvalidFamilyConfig);

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

    std::filesystem::path invalidFrontViewModeConfig = writeConfigFile(
        "airtrace_front_view_invalid_mode.cfg",
        "config.version=1.0\n"
        "front_view.enabled=true\n"
        "front_view.display_families=eo_gray,invalid_mode\n"
        "front_view.spoof.enabled=true\n");
    ConfigResult invalidFrontViewModeResult = loadSimConfig(invalidFrontViewModeConfig.string());
    assert(!invalidFrontViewModeResult.ok);
    std::filesystem::remove(invalidFrontViewModeConfig);

    std::filesystem::path invalidFrontViewThreadingConfig = writeConfigFile(
        "airtrace_front_view_invalid_threading.cfg",
        "config.version=1.0\n"
        "front_view.enabled=true\n"
        "front_view.display_families=eo_gray\n"
        "front_view.threading.enabled=false\n"
        "front_view.threading.max_workers=4\n"
        "front_view.multi_view.max_streams=2\n"
        "front_view.multi_view.stream_ids=primary,turret\n");
    ConfigResult invalidFrontViewThreadingResult = loadSimConfig(invalidFrontViewThreadingConfig.string());
    assert(!invalidFrontViewThreadingResult.ok);
    std::filesystem::remove(invalidFrontViewThreadingConfig);

    std::filesystem::path invalidFrontViewStabilizationConfig = writeConfigFile(
        "airtrace_front_view_invalid_stabilization.cfg",
        "config.version=1.0\n"
        "front_view.enabled=true\n"
        "front_view.display_families=eo_gray\n"
        "front_view.stabilization.enabled=true\n"
        "front_view.stabilization.mode=off\n");
    ConfigResult invalidFrontViewStabilizationResult = loadSimConfig(invalidFrontViewStabilizationConfig.string());
    assert(!invalidFrontViewStabilizationResult.ok);
    std::filesystem::remove(invalidFrontViewStabilizationConfig);

    std::filesystem::path invalidFrontViewGimbalConfig = writeConfigFile(
        "airtrace_front_view_invalid_gimbal.cfg",
        "config.version=1.0\n"
        "front_view.enabled=true\n"
        "front_view.display_families=eo_gray\n"
        "front_view.stabilization.enabled=false\n"
        "front_view.stabilization.mode=off\n"
        "front_view.gimbal.enabled=true\n"
        "front_view.gimbal.max_yaw_rate_deg_s=180\n"
        "front_view.gimbal.max_pitch_rate_deg_s=120\n");
    ConfigResult invalidFrontViewGimbalResult = loadSimConfig(invalidFrontViewGimbalConfig.string());
    assert(!invalidFrontViewGimbalResult.ok);
    std::filesystem::remove(invalidFrontViewGimbalConfig);

    std::filesystem::path validFrontViewConfig = writeConfigFile(
        "airtrace_front_view_valid.cfg",
        "config.version=1.0\n"
        "front_view.enabled=true\n"
        "front_view.display_families=eo_gray,ir_white_hot,proximity_2d\n"
        "front_view.auto_cycle.enabled=true\n"
        "front_view.auto_cycle.interval_ms=1000\n"
        "front_view.auto_cycle.order=eo_gray,ir_white_hot,proximity_2d\n"
        "front_view.render.latency_budget_ms=120\n"
        "front_view.proximity.max_range_m=1500\n"
        "front_view.frame.max_age_ms=500\n"
        "front_view.frame.min_confidence=0.3\n"
        "front_view.multi_view.max_streams=2\n"
        "front_view.multi_view.stream_ids=primary,turret\n"
        "front_view.stabilization.enabled=true\n"
        "front_view.stabilization.mode=gimbal_lock\n"
        "front_view.gimbal.enabled=true\n"
        "front_view.gimbal.max_yaw_rate_deg_s=220\n"
        "front_view.gimbal.max_pitch_rate_deg_s=160\n"
        "front_view.spoof.enabled=true\n"
        "front_view.spoof.pattern=gradient\n"
        "front_view.spoof.motion_profile=linear\n"
        "front_view.spoof.seed=42\n"
        "front_view.spoof.rate_hz=20\n"
        "front_view.security.require_signed_assets=true\n"
        "front_view.threading.enabled=true\n"
        "front_view.threading.max_workers=2\n");
    ConfigResult validFrontViewResult = loadSimConfig(validFrontViewConfig.string());
    assert(validFrontViewResult.ok);
    std::filesystem::remove(validFrontViewConfig);

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
    assert(visionMeas.provenance == ProvenanceTag::Operational);

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
    assert(celestialMeas.provenance == ProvenanceTag::Operational);

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

    ModeManagerConfig unknownProvenanceConfig = provenanceModeConfig;
    unknownProvenanceConfig.allowedProvenances = {ProvenanceTag::Operational};
    unknownProvenanceConfig.provenanceUnknownAction = UnknownProvenanceAction::Deny;
    ModeManager unknownProvenanceDenyManager(unknownProvenanceConfig);
    gps.setProvenance(ProvenanceTag::Unknown);
    gps.setHealthy(true);
    decision = unknownProvenanceDenyManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);
    assert(unknownProvenanceDenyManager.getLastDecisionDetail().downgradeReason == "provenance_unknown");

    unknownProvenanceConfig.provenanceUnknownAction = UnknownProvenanceAction::Hold;
    ModeManager unknownProvenanceHoldManager(unknownProvenanceConfig);
    decision = unknownProvenanceHoldManager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);
    assert(unknownProvenanceHoldManager.getLastDecisionDetail().downgradeReason == "provenance_unknown_hold");

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

    ExternalIoEnvelope packagerEnvelope;
    packagerEnvelope.metadata.schemaVersion = "1.0.0";
    packagerEnvelope.metadata.interfaceId = "airtrace.external_io";
    packagerEnvelope.metadata.platformProfile = "air";
    packagerEnvelope.metadata.adapterId = "official.air";
    packagerEnvelope.metadata.adapterVersion = "1.0.0";
    packagerEnvelope.metadata.uiSurface = "tui";
    packagerEnvelope.metadata.seed = 42U;
    packagerEnvelope.metadata.deterministic = true;
    packagerEnvelope.mode.activeMode = "gps";
    packagerEnvelope.mode.contributors = {"gps", "imu"};
    packagerEnvelope.mode.confidence = 0.9101234567890123;
    packagerEnvelope.mode.decisionReason = "gps_eligible";
    packagerEnvelope.mode.denialReason.clear();
    packagerEnvelope.mode.ladderStatus = "ok";
    packagerEnvelope.sensors.push_back({"gps", true, true, true, 0.10000000000000001, 0.9501234567890123, ""});
    packagerEnvelope.frontView.frameAgeMs = 1.234567890123456;
    packagerEnvelope.frontView.acquisitionLatencyMs = 2.345678901234567;
    packagerEnvelope.frontView.processingLatencyMs = 3.456789012345678;
    packagerEnvelope.frontView.renderLatencyMs = 4.567890123456789;
    packagerEnvelope.frontView.latencyMs = 10.987654321012345;
    packagerEnvelope.frontView.confidence = 0.8123456789012345;
    packagerEnvelope.frontView.stabilizationErrorDeg = 0.000000123456789;
    packagerEnvelope.frontView.gimbalYawDeg = 12.3456789012345;
    packagerEnvelope.frontView.gimbalPitchDeg = -6.54321098765432;
    packagerEnvelope.frontView.gimbalYawRateDegPerSec = 0.3333333333333333;
    packagerEnvelope.frontView.gimbalPitchRateDegPerSec = -0.2222222222222222;
    packagerEnvelope.frontView.streamCount = 0;
    packagerEnvelope.frontView.streamIndex = 0;
    packagerEnvelope.frontView.maxConcurrentViews = 1;

    tools::IoEnvelopeSerializeResult serializedJson =
        tools::serializeExternalIoEnvelope(tools::IoEnvelopeFormat::Json, packagerEnvelope);
    assert(serializedJson.ok);
    tools::IoEnvelopeParseResult parsedJson =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::Json, serializedJson.payload);
    assert(parsedJson.ok);
    assert(parsedJson.envelope.metadata.platformProfile == "air");
    assert(parsedJson.envelope.mode.activeMode == "gps");
    assert(parsedJson.envelope.sensors.size() == 1);
    assert(parsedJson.envelope.sensors[0].sensorId == "gps");
    assert(parsedJson.envelope.mode.contributors.size() == 2);
    assert(parsedJson.envelope.mode.confidence == packagerEnvelope.mode.confidence);
    assert(parsedJson.envelope.frontView.latencyMs == packagerEnvelope.frontView.latencyMs);
    assert(parsedJson.envelope.frontView.gimbalYawDeg == packagerEnvelope.frontView.gimbalYawDeg);

    tools::IoEnvelopeSerializeResult serializedKv =
        tools::serializeExternalIoEnvelope(tools::IoEnvelopeFormat::KeyValue, packagerEnvelope);
    assert(serializedKv.ok);
    tools::IoEnvelopeParseResult parsedKv =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::KeyValue, serializedKv.payload);
    assert(parsedKv.ok);
    assert(parsedKv.envelope.metadata.seed == 42U);
    assert(parsedKv.envelope.sensors[0].healthy);
    assert(parsedKv.envelope.mode.confidence == packagerEnvelope.mode.confidence);
    assert(parsedKv.envelope.frontView.confidence == packagerEnvelope.frontView.confidence);

    tools::IoEnvelopeSerializeResult convertedKv =
        tools::convertExternalIoEnvelope(
            serializedJson.payload,
            tools::IoEnvelopeFormat::Json,
            tools::IoEnvelopeFormat::KeyValue);
    assert(convertedKv.ok);
    tools::IoEnvelopeParseResult convertedParsed =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::KeyValue, convertedKv.payload);
    assert(convertedParsed.ok);
    assert(convertedParsed.envelope.mode.activeMode == "gps");

    std::vector<tools::IoEnvelopeCodecDescriptor> codecs = tools::listIoEnvelopeCodecs();
    assert(codecs.size() >= 2U);
    bool sawJsonCodec = false;
    bool sawKvCodec = false;
    for (const auto &codec : codecs)
    {
        if (codec.canonicalName == "ie_json_v1")
        {
            sawJsonCodec = true;
        }
        if (codec.canonicalName == "ie_kv_v1")
        {
            sawKvCodec = true;
        }
    }
    assert(sawJsonCodec);
    assert(sawKvCodec);
    assert(tools::isSupportedIoEnvelopeFormat("json"));
    assert(tools::isSupportedIoEnvelopeFormat("ie_json_v1"));
    assert(tools::isSupportedIoEnvelopeFormat("kv"));
    assert(tools::isSupportedIoEnvelopeFormat("ie_kv_v1"));
    assert(!tools::isSupportedIoEnvelopeFormat("xml"));

    tools::IoEnvelopeParseResult parsedByName =
        tools::parseExternalIoEnvelope("ie_json_v1", serializedJson.payload);
    assert(parsedByName.ok);
    assert(parsedByName.envelope.metadata.interfaceId == packagerEnvelope.metadata.interfaceId);
    tools::IoEnvelopeSerializeResult serializedByName =
        tools::serializeExternalIoEnvelope("kv", parsedByName.envelope);
    assert(serializedByName.ok);
    tools::IoEnvelopeSerializeResult convertedByName =
        tools::convertExternalIoEnvelope(serializedByName.payload, "ie_kv_v1", "ie_json_v1");
    assert(convertedByName.ok);
    tools::IoEnvelopeSerializeResult unsupportedFormat =
        tools::convertExternalIoEnvelope(serializedByName.payload, "ie_kv_v1", "yaml");
    assert(!unsupportedFormat.ok);

    std::string numericJsonPayload = serializedJson.payload;
    const auto replaceJsonQuotedValue = [](std::string &text, const std::string &key, const std::string &literal) -> bool
    {
        const std::string prefix = "\"" + key + "\":\"";
        const std::size_t keyPos = text.find(prefix);
        if (keyPos == std::string::npos)
        {
            return false;
        }
        const std::size_t valueStart = keyPos + prefix.size();
        const std::size_t valueEnd = text.find('"', valueStart);
        if (valueEnd == std::string::npos)
        {
            return false;
        }
        text.replace(keyPos, valueEnd - keyPos + 1, "\"" + key + "\":" + literal);
        return true;
    };
    assert(replaceJsonQuotedValue(numericJsonPayload, "metadata.seed", "42"));
    assert(replaceJsonQuotedValue(numericJsonPayload, "metadata.deterministic", "true"));
    assert(replaceJsonQuotedValue(numericJsonPayload, "mode.confidence", "0.9101234567890123"));
    tools::IoEnvelopeParseResult numericJsonParsed =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::Json, numericJsonPayload);
    assert(numericJsonParsed.ok);
    assert(numericJsonParsed.envelope.metadata.seed == 42U);
    assert(numericJsonParsed.envelope.metadata.deterministic);

    tools::IoEnvelopeParseResult malformedKv =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::KeyValue, "schema_version=1.0.0\n");
    assert(!malformedKv.ok);

    tools::IoEnvelopeParseResult malformedJson =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::Json, "{\"schema_version\":\"1.0.0\"");
    assert(!malformedJson.ok);

    std::string nonFiniteKvPayload = serializedKv.payload;
    const auto replaceKvValue = [](std::string &text, const std::string &key, const std::string &value) -> bool
    {
        const std::string prefix = key + "=";
        const std::size_t keyPos = text.find(prefix);
        if (keyPos == std::string::npos)
        {
            return false;
        }
        const std::size_t valueStart = keyPos + prefix.size();
        const std::size_t valueEnd = text.find('\n', valueStart);
        if (valueEnd == std::string::npos)
        {
            text.replace(valueStart, text.size() - valueStart, value);
            return true;
        }
        text.replace(valueStart, valueEnd - valueStart, value);
        return true;
    };
    assert(replaceKvValue(nonFiniteKvPayload, "front_view.confidence", "nan"));
    tools::IoEnvelopeParseResult nonFiniteKv =
        tools::parseExternalIoEnvelope(tools::IoEnvelopeFormat::KeyValue, nonFiniteKvPayload);
    assert(!nonFiniteKv.ok);

    tools::FederationBridgeConfig bridgeConfig;
    bridgeConfig.startLogicalTick = 100;
    bridgeConfig.tickStep = 5;
    bridgeConfig.startTimestampMs = 1000;
    bridgeConfig.tickDurationMs = 20;
    bridgeConfig.maxLatencyBudgetMs = 2500.0;
    bridgeConfig.requireDeterministic = true;
    bridgeConfig.outputFormatName = "ie_json_v1";
    bridgeConfig.federateId = "edge_node_1";
    bridgeConfig.routeDomain = "theater_alpha";
    bridgeConfig.requireSourceTimestamp = true;
    bridgeConfig.requireMonotonicSourceTimestamp = true;
    bridgeConfig.maxFutureSkewMs = 0;
    bridgeConfig.federateKeyId = "key_alpha";
    tools::FederationBridge bridge(bridgeConfig);

    packagerEnvelope.frontView.timestampMs = 1100;
    packagerEnvelope.frontView.sourceId = "front_sensor";
    tools::FederationBridgeResult bridgeFrame1 = bridge.publish(packagerEnvelope);
    assert(bridgeFrame1.ok);
    assert(bridgeFrame1.frame.logicalTick == 100U);
    assert(bridgeFrame1.frame.eventTimestampMs == 3000U);
    assert(bridgeFrame1.frame.sourceTimestampMs == 1100U);
    assert(bridgeFrame1.frame.sourceLatencyMs == 1900.0);
    assert(bridgeFrame1.frame.federateId == "edge_node_1");
    assert(bridgeFrame1.frame.federateKeyId == "key_alpha");
    assert(bridgeFrame1.frame.endpointId == "endpoint_default");
    assert(bridgeFrame1.frame.routeKey == "theater_alpha/air/front_sensor");
    assert(bridgeFrame1.frame.routeSequence == 0U);
    assert(bridgeFrame1.frame.payloadFormat == "ie_json_v1");
    tools::IoEnvelopeParseResult bridgePayloadParsed =
        tools::parseExternalIoEnvelope(bridgeFrame1.frame.payloadFormat, bridgeFrame1.frame.payload);
    assert(bridgePayloadParsed.ok);
    assert(bridgePayloadParsed.envelope.mode.activeMode == packagerEnvelope.mode.activeMode);
    const std::string bridgeJson = tools::serializeFederationEventFrameJson(bridgeFrame1.frame);
    assert(bridgeJson.find("\"logical_tick\":100") != std::string::npos);
    assert(bridgeJson.find("\"payload_format\":\"ie_json_v1\"") != std::string::npos);
    assert(bridgeJson.find("\"endpoint_id\":\"endpoint_default\"") != std::string::npos);

    tools::FederationBridgeResult bridgeFrame2 = bridge.publish(packagerEnvelope);
    assert(bridgeFrame2.ok);
    assert(bridgeFrame2.frame.logicalTick == 105U);
    assert(bridgeFrame2.frame.eventTimestampMs == 3100U);
    assert(bridgeFrame2.frame.routeSequence == 1U);

    tools::FederationBridgeConfig unsupportedCodecConfig = bridgeConfig;
    unsupportedCodecConfig.outputFormatName = "yaml";
    tools::FederationBridge unsupportedBridge(unsupportedCodecConfig);
    tools::FederationBridgeResult unsupportedCodecResult = unsupportedBridge.publish(packagerEnvelope);
    assert(!unsupportedCodecResult.ok);

    tools::FederationBridgeConfig invalidEndpointCodecConfig = bridgeConfig;
    invalidEndpointCodecConfig.endpoints = {{"edge_a", "yaml", true}};
    tools::FederationBridge invalidEndpointCodecBridge(invalidEndpointCodecConfig);
    tools::FederationBridgeResult invalidEndpointCodecResult = invalidEndpointCodecBridge.publish(packagerEnvelope);
    assert(!invalidEndpointCodecResult.ok);

    tools::FederationBridgeConfig nondeterministicRequiredConfig = bridgeConfig;
    nondeterministicRequiredConfig.requireDeterministic = true;
    tools::FederationBridge nondeterministicBridge(nondeterministicRequiredConfig);
    ExternalIoEnvelope nondeterministicEnvelope = packagerEnvelope;
    nondeterministicEnvelope.metadata.deterministic = false;
    tools::FederationBridgeResult nondeterministicResult = nondeterministicBridge.publish(nondeterministicEnvelope);
    assert(!nondeterministicResult.ok);

    tools::FederationBridgeConfig lowLatencyConfig = bridgeConfig;
    lowLatencyConfig.maxLatencyBudgetMs = 10.0;
    tools::FederationBridge lowLatencyBridge(lowLatencyConfig);
    tools::FederationBridgeResult lowLatencyResult = lowLatencyBridge.publish(packagerEnvelope);
    assert(!lowLatencyResult.ok);

    tools::FederationBridgeConfig invalidTickConfig = bridgeConfig;
    invalidTickConfig.tickStep = 0;
    tools::FederationBridge invalidTickBridge(invalidTickConfig);
    tools::FederationBridgeResult invalidTickResult = invalidTickBridge.publish(packagerEnvelope);
    assert(!invalidTickResult.ok);

    tools::FederationBridgeConfig allowedSourceConfig = bridgeConfig;
    allowedSourceConfig.allowedSourceIds = {"front_sensor"};
    tools::FederationBridge allowedSourceBridge(allowedSourceConfig);
    tools::FederationBridgeResult allowedSourceResult = allowedSourceBridge.publish(packagerEnvelope);
    assert(allowedSourceResult.ok);
    allowedSourceConfig.allowedSourceIds = {"other_source"};
    tools::FederationBridge deniedSourceBridge(allowedSourceConfig);
    tools::FederationBridgeResult deniedSourceResult = deniedSourceBridge.publish(packagerEnvelope);
    assert(!deniedSourceResult.ok);

    tools::FederationBridgeConfig missingSourceTimestampConfig = bridgeConfig;
    missingSourceTimestampConfig.requireSourceTimestamp = true;
    tools::FederationBridge missingSourceTimestampBridge(missingSourceTimestampConfig);
    ExternalIoEnvelope missingSourceTimestampEnvelope = packagerEnvelope;
    missingSourceTimestampEnvelope.frontView.timestampMs = 0U;
    tools::FederationBridgeResult missingSourceTimestampResult =
        missingSourceTimestampBridge.publish(missingSourceTimestampEnvelope);
    assert(!missingSourceTimestampResult.ok);

    tools::FederationBridgeConfig futureSkewConfig = bridgeConfig;
    futureSkewConfig.maxFutureSkewMs = 50U;
    tools::FederationBridge futureSkewBridge(futureSkewConfig);
    ExternalIoEnvelope futureSkewEnvelope = packagerEnvelope;
    futureSkewEnvelope.frontView.timestampMs = 3100U;
    tools::FederationBridgeResult futureSkewResult = futureSkewBridge.publish(futureSkewEnvelope);
    assert(!futureSkewResult.ok);

    tools::FederationBridgeConfig monotonicConfig = bridgeConfig;
    monotonicConfig.maxFutureSkewMs = 1000U;
    tools::FederationBridge monotonicBridge(monotonicConfig);
    ExternalIoEnvelope monotonicEnvelope = packagerEnvelope;
    monotonicEnvelope.frontView.timestampMs = 1050U;
    tools::FederationBridgeResult monotonicFirst = monotonicBridge.publish(monotonicEnvelope);
    assert(monotonicFirst.ok);
    monotonicEnvelope.frontView.timestampMs = 1040U;
    tools::FederationBridgeResult monotonicSecond = monotonicBridge.publish(monotonicEnvelope);
    assert(!monotonicSecond.ok);

    tools::FederationBridgeConfig fanoutConfig = bridgeConfig;
    fanoutConfig.startLogicalTick = 0;
    fanoutConfig.tickStep = 1;
    fanoutConfig.startTimestampMs = 0;
    fanoutConfig.tickDurationMs = 10;
    fanoutConfig.maxLatencyBudgetMs = 1000.0;
    fanoutConfig.maxFutureSkewMs = 1000U;
    fanoutConfig.requireSourceTimestamp = false;
    fanoutConfig.endpoints = {
        {"edge_a", "ie_json_v1", true},
        {"edge_b", "ie_kv_v1", true}};
    tools::FederationBridge fanoutBridge(fanoutConfig);
    ExternalIoEnvelope fanoutEnvelope = packagerEnvelope;
    fanoutEnvelope.frontView.timestampMs = 0U;
    tools::FederationFanoutResult fanoutResult = fanoutBridge.publishFanout(fanoutEnvelope);
    assert(fanoutResult.ok);
    assert(fanoutResult.frames.size() == 2U);
    assert(fanoutResult.frames[0].routeSequence == 0U);
    assert(fanoutResult.frames[1].routeSequence == 0U);
    assert(fanoutResult.frames[0].endpointId == "edge_a");
    assert(fanoutResult.frames[1].endpointId == "edge_b");
    tools::IoEnvelopeParseResult fanoutParsedA =
        tools::parseExternalIoEnvelope(fanoutResult.frames[0].payloadFormat, fanoutResult.frames[0].payload);
    assert(fanoutParsedA.ok);
    tools::IoEnvelopeParseResult fanoutParsedB =
        tools::parseExternalIoEnvelope(fanoutResult.frames[1].payloadFormat, fanoutResult.frames[1].payload);
    assert(fanoutParsedB.ok);
    tools::FederationFanoutResult fanoutResult2 = fanoutBridge.publishFanout(fanoutEnvelope);
    assert(fanoutResult2.ok);
    assert(fanoutResult2.frames[0].routeSequence == 1U);
    assert(fanoutResult2.frames[1].routeSequence == 1U);

    return 0;
}
