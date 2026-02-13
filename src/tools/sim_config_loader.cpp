#include "tools/sim_config_loader.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <unordered_map>

#include "core/plugin_auth.h"
#include "tools/adapter_registry_loader.h"

namespace
{
constexpr const char *kExpectedVersion = "1.0";

std::string trim(const std::string &value)
{
    size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

std::vector<std::string> splitList(const std::string &value)
{
    std::vector<std::string> items;
    std::stringstream ss(value);
    std::string part;
    while (std::getline(ss, part, ','))
    {
        std::string trimmed = toLower(trim(part));
        if (!trimmed.empty())
        {
            items.push_back(trimmed);
        }
    }
    return items;
}

std::vector<std::string> defaultSensorsForProfile(SimConfig::PlatformProfile profile)
{
    switch (profile)
    {
    case SimConfig::PlatformProfile::Air:
        return {"gps", "imu", "baro", "magnetometer", "radar", "thermal", "vision", "lidar", "celestial", "dead_reckoning"};
    case SimConfig::PlatformProfile::Ground:
        return {"gps", "imu", "vision", "lidar", "radar", "thermal", "magnetometer", "baro", "celestial", "dead_reckoning"};
    case SimConfig::PlatformProfile::Maritime:
        return {"gps", "imu", "radar", "magnetometer", "baro", "vision", "celestial", "dead_reckoning"};
    case SimConfig::PlatformProfile::Space:
        return {"gps", "imu", "celestial", "dead_reckoning"};
    case SimConfig::PlatformProfile::Handheld:
        return {"gps", "imu", "magnetometer", "baro", "vision", "celestial", "dead_reckoning"};
    case SimConfig::PlatformProfile::FixedSite:
        return {"gps", "imu", "celestial"};
    case SimConfig::PlatformProfile::Subsea:
        return {"imu", "baro", "magnetometer", "dead_reckoning"};
    case SimConfig::PlatformProfile::Base:
    default:
        return {"gps", "imu", "thermal", "radar", "dead_reckoning"};
    }
}

void appendUnique(std::vector<std::string> &target, const std::vector<std::string> &source)
{
    for (const auto &entry : source)
    {
        if (std::find(target.begin(), target.end(), entry) == target.end())
        {
            target.push_back(entry);
        }
    }
}

bool toDouble(const std::string &value, double &out)
{
    char *end = nullptr;
    out = std::strtod(value.c_str(), &end);
    return end && *end == '\0';
}

bool toInt(const std::string &value, int &out)
{
    char *end = nullptr;
    long temp = std::strtol(value.c_str(), &end, 10);
    if (!end || *end != '\0')
    {
        return false;
    }
    out = static_cast<int>(temp);
    return true;
}

bool toUnsigned(const std::string &value, unsigned int &out)
{
    char *end = nullptr;
    unsigned long temp = std::strtoul(value.c_str(), &end, 10);
    if (!end || *end != '\0')
    {
        return false;
    }
    out = static_cast<unsigned int>(temp);
    return true;
}

bool toBool(const std::string &value, bool &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "true" || lowered == "1" || lowered == "yes")
    {
        out = true;
        return true;
    }
    if (lowered == "false" || lowered == "0" || lowered == "no")
    {
        out = false;
        return true;
    }
    return false;
}

bool toProfile(const std::string &value, SimConfig::PlatformProfile &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "base") out = SimConfig::PlatformProfile::Base;
    else if (lowered == "air") out = SimConfig::PlatformProfile::Air;
    else if (lowered == "ground") out = SimConfig::PlatformProfile::Ground;
    else if (lowered == "maritime") out = SimConfig::PlatformProfile::Maritime;
    else if (lowered == "space") out = SimConfig::PlatformProfile::Space;
    else if (lowered == "handheld") out = SimConfig::PlatformProfile::Handheld;
    else if (lowered == "fixed_site") out = SimConfig::PlatformProfile::FixedSite;
    else if (lowered == "subsea") out = SimConfig::PlatformProfile::Subsea;
    else return false;
    return true;
}

bool isValidModuleName(const std::string &name)
{
    if (name.empty())
    {
        return false;
    }
    for (char ch : name)
    {
        if (!(std::islower(static_cast<unsigned char>(ch)) || std::isdigit(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-'))
        {
            return false;
        }
    }
    return true;
}

bool isValidAdapterId(const std::string &name)
{
    return isValidModuleName(name);
}

bool isOfficialAdapterId(const std::string &name)
{
    return name == "air" || name == "ground" || name == "maritime" ||
           name == "space" || name == "handheld" || name == "fixed_site" ||
           name == "subsea";
}

bool isValidModeName(const std::string &name)
{
    static const std::vector<std::string> kModes = {
        "gps_ins",
        "gps",
        "vio",
        "lio",
        "radar_inertial",
        "thermal",
        "radar",
        "vision",
        "lidar",
        "mag_baro",
        "magnetometer",
        "baro",
        "celestial",
        "dead_reckoning",
        "imu",
        "hold"};
    for (const auto &mode : kModes)
    {
        if (mode == name)
        {
            return true;
        }
    }
    return false;
}

bool isSemver(const std::string &value)
{
    int major = 0;
    int minor = 0;
    int patch = 0;
    char dot1 = '\0';
    char dot2 = '\0';
    std::istringstream input(trim(value));
    if (!(input >> major >> dot1 >> minor >> dot2 >> patch))
    {
        return false;
    }
    if (dot1 != '.' || dot2 != '.')
    {
        return false;
    }
    if (major < 0 || minor < 0 || patch < 0)
    {
        return false;
    }
    char trailing = '\0';
    if (input >> trailing)
    {
        return false;
    }
    return true;
}

bool isSha256Hex(const std::string &value)
{
    if (value.size() != 64)
    {
        return false;
    }
    for (char ch : value)
    {
        if (!std::isdigit(static_cast<unsigned char>(ch)) &&
            !(ch >= 'a' && ch <= 'f') &&
            !(ch >= 'A' && ch <= 'F'))
        {
            return false;
        }
    }
    return true;
}

bool toNetworkAidMode(const std::string &value, SimConfig::NetworkAidMode &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "deny") out = SimConfig::NetworkAidMode::Deny;
    else if (lowered == "allow") out = SimConfig::NetworkAidMode::Allow;
    else if (lowered == "test_only") out = SimConfig::NetworkAidMode::TestOnly;
    else return false;
    return true;
}

bool toOverrideAuth(const std::string &value, SimConfig::OverrideAuth &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "credential") out = SimConfig::OverrideAuth::Credential;
    else if (lowered == "key") out = SimConfig::OverrideAuth::Key;
    else if (lowered == "token") out = SimConfig::OverrideAuth::Token;
    else return false;
    return true;
}

bool toDatasetTier(const std::string &value, SimConfig::DatasetTier &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "minimal") out = SimConfig::DatasetTier::Minimal;
    else if (lowered == "standard") out = SimConfig::DatasetTier::Standard;
    else if (lowered == "extended") out = SimConfig::DatasetTier::Extended;
    else return false;
    return true;
}

bool toProvenanceMode(const std::string &value, SimConfig::ProvenanceMode &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "operational") out = SimConfig::ProvenanceMode::Operational;
    else if (lowered == "simulation") out = SimConfig::ProvenanceMode::Simulation;
    else if (lowered == "test") out = SimConfig::ProvenanceMode::Test;
    else return false;
    return true;
}

bool toUnknownProvenanceAction(const std::string &value, SimConfig::UnknownProvenanceAction &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "deny") out = SimConfig::UnknownProvenanceAction::Deny;
    else if (lowered == "hold") out = SimConfig::UnknownProvenanceAction::Hold;
    else return false;
    return true;
}

bool toUiSurface(const std::string &value, std::string &out)
{
    std::string lowered = toLower(trim(value));
    if (lowered == "tui" || lowered == "cockpit" || lowered == "remote_operator" || lowered == "c2")
    {
        out = lowered;
        return true;
    }
    return false;
}

void setIssue(ConfigResult &result, const std::string &key, const std::string &message)
{
    result.ok = false;
    result.issues.push_back({key, message});
}

void applyValue(SimConfig &config, ConfigResult &result, const std::string &key, const std::string &value)
{
    double dval = 0.0;
    int ival = 0;
    unsigned int uval = 0;
    bool bval = false;
    SimConfig::PlatformProfile profile = SimConfig::PlatformProfile::Base;
    SimConfig::NetworkAidMode aidMode = SimConfig::NetworkAidMode::Deny;
    SimConfig::OverrideAuth authMode = SimConfig::OverrideAuth::Credential;
    SimConfig::DatasetTier tier = SimConfig::DatasetTier::Minimal;
    SimConfig::ProvenanceMode provenanceMode = SimConfig::ProvenanceMode::Operational;
    SimConfig::UnknownProvenanceAction unknownAction = SimConfig::UnknownProvenanceAction::Deny;
    std::string uiSurface;

    if (key == "config.version")
    {
        config.version = value;
    }
    else if (key == "state.position.x" && toDouble(value, dval)) config.initialState.position.x = dval;
    else if (key == "state.position.y" && toDouble(value, dval)) config.initialState.position.y = dval;
    else if (key == "state.position.z" && toDouble(value, dval)) config.initialState.position.z = dval;
    else if (key == "state.velocity.x" && toDouble(value, dval)) config.initialState.velocity.x = dval;
    else if (key == "state.velocity.y" && toDouble(value, dval)) config.initialState.velocity.y = dval;
    else if (key == "state.velocity.z" && toDouble(value, dval)) config.initialState.velocity.z = dval;
    else if (key == "state.acceleration.x" && toDouble(value, dval)) config.initialState.acceleration.x = dval;
    else if (key == "state.acceleration.y" && toDouble(value, dval)) config.initialState.acceleration.y = dval;
    else if (key == "state.acceleration.z" && toDouble(value, dval)) config.initialState.acceleration.z = dval;
    else if (key == "state.time" && toDouble(value, dval)) config.initialState.time = dval;
    else if (key == "sim.dt" && toDouble(value, dval)) config.dt = dval;
    else if (key == "sim.steps" && toInt(value, ival)) config.steps = ival;
    else if (key == "sim.seed" && toUnsigned(value, uval)) config.seed = uval;
    else if (key == "bounds.min.x" && toDouble(value, dval)) config.bounds.minPosition.x = dval;
    else if (key == "bounds.min.y" && toDouble(value, dval)) config.bounds.minPosition.y = dval;
    else if (key == "bounds.min.z" && toDouble(value, dval)) config.bounds.minPosition.z = dval;
    else if (key == "bounds.max.x" && toDouble(value, dval)) config.bounds.maxPosition.x = dval;
    else if (key == "bounds.max.y" && toDouble(value, dval)) config.bounds.maxPosition.y = dval;
    else if (key == "bounds.max.z" && toDouble(value, dval)) config.bounds.maxPosition.z = dval;
    else if (key == "bounds.max_speed" && toDouble(value, dval)) config.bounds.maxSpeed = dval;
    else if (key == "bounds.max_accel" && toDouble(value, dval)) config.bounds.maxAcceleration = dval;
    else if (key == "bounds.max_turn_rate_deg" && toDouble(value, dval)) config.bounds.maxTurnRateDeg = dval;
    else if (key == "maneuver.random_accel_std" && toDouble(value, dval)) config.maneuvers.randomAccelStd = dval;
    else if (key == "maneuver.probability" && toDouble(value, dval)) config.maneuvers.maneuverProbability = dval;
    else if (key == "sensor.gps.rate_hz" && toDouble(value, dval)) config.gps.rateHz = dval;
    else if (key == "sensor.gps.noise_std" && toDouble(value, dval)) config.gps.noiseStd = dval;
    else if (key == "sensor.gps.dropout" && toDouble(value, dval)) config.gps.dropoutProbability = dval;
    else if (key == "sensor.gps.false_positive" && toDouble(value, dval)) config.gps.falsePositiveProbability = dval;
    else if (key == "sensor.gps.max_range" && toDouble(value, dval)) config.gps.maxRange = dval;
    else if (key == "sensor.thermal.rate_hz" && toDouble(value, dval)) config.thermal.rateHz = dval;
    else if (key == "sensor.thermal.noise_std" && toDouble(value, dval)) config.thermal.noiseStd = dval;
    else if (key == "sensor.thermal.dropout" && toDouble(value, dval)) config.thermal.dropoutProbability = dval;
    else if (key == "sensor.thermal.false_positive" && toDouble(value, dval)) config.thermal.falsePositiveProbability = dval;
    else if (key == "sensor.thermal.max_range" && toDouble(value, dval)) config.thermal.maxRange = dval;
    else if (key == "sensor.dead_reckoning.rate_hz" && toDouble(value, dval)) config.deadReckoning.rateHz = dval;
    else if (key == "sensor.dead_reckoning.noise_std" && toDouble(value, dval)) config.deadReckoning.noiseStd = dval;
    else if (key == "sensor.dead_reckoning.dropout" && toDouble(value, dval)) config.deadReckoning.dropoutProbability = dval;
    else if (key == "sensor.dead_reckoning.false_positive" && toDouble(value, dval)) config.deadReckoning.falsePositiveProbability = dval;
    else if (key == "sensor.dead_reckoning.max_range" && toDouble(value, dval)) config.deadReckoning.maxRange = dval;
    else if (key == "sensor.imu.rate_hz" && toDouble(value, dval)) config.imu.rateHz = dval;
    else if (key == "sensor.imu.noise_std" && toDouble(value, dval)) config.imu.noiseStd = dval;
    else if (key == "sensor.imu.dropout" && toDouble(value, dval)) config.imu.dropoutProbability = dval;
    else if (key == "sensor.imu.false_positive" && toDouble(value, dval)) config.imu.falsePositiveProbability = dval;
    else if (key == "sensor.imu.max_range" && toDouble(value, dval)) config.imu.maxRange = dval;
    else if (key == "sensor.radar.rate_hz" && toDouble(value, dval)) config.radar.rateHz = dval;
    else if (key == "sensor.radar.noise_std" && toDouble(value, dval)) config.radar.noiseStd = dval;
    else if (key == "sensor.radar.dropout" && toDouble(value, dval)) config.radar.dropoutProbability = dval;
    else if (key == "sensor.radar.false_positive" && toDouble(value, dval)) config.radar.falsePositiveProbability = dval;
    else if (key == "sensor.radar.max_range" && toDouble(value, dval)) config.radar.maxRange = dval;
    else if (key == "sensor.vision.rate_hz" && toDouble(value, dval)) config.vision.rateHz = dval;
    else if (key == "sensor.vision.noise_std" && toDouble(value, dval)) config.vision.noiseStd = dval;
    else if (key == "sensor.vision.dropout" && toDouble(value, dval)) config.vision.dropoutProbability = dval;
    else if (key == "sensor.vision.false_positive" && toDouble(value, dval)) config.vision.falsePositiveProbability = dval;
    else if (key == "sensor.vision.max_range" && toDouble(value, dval)) config.vision.maxRange = dval;
    else if (key == "sensor.lidar.rate_hz" && toDouble(value, dval)) config.lidar.rateHz = dval;
    else if (key == "sensor.lidar.noise_std" && toDouble(value, dval)) config.lidar.noiseStd = dval;
    else if (key == "sensor.lidar.dropout" && toDouble(value, dval)) config.lidar.dropoutProbability = dval;
    else if (key == "sensor.lidar.false_positive" && toDouble(value, dval)) config.lidar.falsePositiveProbability = dval;
    else if (key == "sensor.lidar.max_range" && toDouble(value, dval)) config.lidar.maxRange = dval;
    else if (key == "sensor.magnetometer.rate_hz" && toDouble(value, dval)) config.magnetometer.rateHz = dval;
    else if (key == "sensor.magnetometer.noise_std" && toDouble(value, dval)) config.magnetometer.noiseStd = dval;
    else if (key == "sensor.magnetometer.dropout" && toDouble(value, dval)) config.magnetometer.dropoutProbability = dval;
    else if (key == "sensor.magnetometer.false_positive" && toDouble(value, dval)) config.magnetometer.falsePositiveProbability = dval;
    else if (key == "sensor.magnetometer.max_range" && toDouble(value, dval)) config.magnetometer.maxRange = dval;
    else if (key == "sensor.baro.rate_hz" && toDouble(value, dval)) config.baro.rateHz = dval;
    else if (key == "sensor.baro.noise_std" && toDouble(value, dval)) config.baro.noiseStd = dval;
    else if (key == "sensor.baro.dropout" && toDouble(value, dval)) config.baro.dropoutProbability = dval;
    else if (key == "sensor.baro.false_positive" && toDouble(value, dval)) config.baro.falsePositiveProbability = dval;
    else if (key == "sensor.baro.max_range" && toDouble(value, dval)) config.baro.maxRange = dval;
    else if (key == "sensor.celestial.rate_hz" && toDouble(value, dval)) config.celestial.rateHz = dval;
    else if (key == "sensor.celestial.noise_std" && toDouble(value, dval)) config.celestial.noiseStd = dval;
    else if (key == "sensor.celestial.dropout" && toDouble(value, dval)) config.celestial.dropoutProbability = dval;
    else if (key == "sensor.celestial.false_positive" && toDouble(value, dval)) config.celestial.falsePositiveProbability = dval;
    else if (key == "sensor.celestial.max_range" && toDouble(value, dval)) config.celestial.maxRange = dval;
    else if (key == "platform.profile" && toProfile(value, profile)) config.platformProfile = profile;
    else if (key == "platform.profile_parent" && toProfile(value, profile))
    {
        config.parentProfile = profile;
        config.hasParentProfile = true;
    }
    else if (key == "platform.permitted_sensors") config.permittedSensors = splitList(value);
    else if (key == "platform.child_modules") config.childModules = splitList(value);
    else if (key == "policy.network_aid.mode" && toNetworkAidMode(value, aidMode)) config.policy.networkAidMode = aidMode;
    else if (key == "policy.network_aid.override_required" && toBool(value, bval)) config.policy.overrideRequired = bval;
    else if (key == "policy.network_aid.override_auth" && toOverrideAuth(value, authMode)) config.policy.overrideAuth = authMode;
    else if (key == "policy.network_aid.override_timeout_seconds" && toInt(value, ival)) config.policy.overrideTimeoutSeconds = ival;
    else if (key == "policy.roles") config.policy.roles = splitList(value);
    else if (key == "policy.active_role") config.policy.activeRole = toLower(trim(value));
    else if (key == "policy.authorization.version") config.policy.authorization.version = value;
    else if (key == "policy.authorization.source") config.policy.authorization.source = value;
    else if (key == "policy.authorization.allowed_modes") config.policy.authorization.allowedModes = splitList(value);
    else if (key == "provenance.run_mode" && toProvenanceMode(value, provenanceMode)) config.provenance.runMode = provenanceMode;
    else if (key == "provenance.run_mode") setIssue(result, key, "invalid provenance run_mode");
    else if (key == "provenance.allowed_inputs")
    {
        std::vector<std::string> inputs = splitList(value);
        std::vector<SimConfig::ProvenanceMode> parsed;
        parsed.reserve(inputs.size());
        bool ok = true;
        for (const auto &entry : inputs)
        {
            SimConfig::ProvenanceMode mode = SimConfig::ProvenanceMode::Operational;
            if (!toProvenanceMode(entry, mode))
            {
                ok = false;
                break;
            }
            parsed.push_back(mode);
        }
        if (!ok)
        {
            setIssue(result, key, "invalid provenance value");
        }
        else
        {
            config.provenance.allowedInputs = std::move(parsed);
        }
    }
    else if (key == "provenance.allow_mixed" && toBool(value, bval)) config.provenance.allowMixed = bval;
    else if (key == "provenance.allow_mixed") setIssue(result, key, "invalid boolean");
    else if (key == "provenance.unknown_action" && toUnknownProvenanceAction(value, unknownAction)) config.provenance.unknownAction = unknownAction;
    else if (key == "provenance.unknown_action") setIssue(result, key, "invalid unknown_action");
    else if (key.rfind("policy.role_permissions.", 0) == 0)
    {
        std::string role = toLower(trim(key.substr(std::string("policy.role_permissions.").size())));
        if (role.empty())
        {
            setIssue(result, key, "missing role name");
        }
        else
        {
            config.policy.rolePermissions[role] = splitList(value);
        }
    }
    else if (key == "dataset.celestial.tier" && toDatasetTier(value, tier)) config.dataset.tier = tier;
    else if (key == "dataset.celestial.max_size_mb" && toDouble(value, dval)) config.dataset.maxSizeMB = dval;
    else if (key == "dataset.celestial.catalog_path") config.dataset.celestialCatalogPath = value;
    else if (key == "dataset.celestial.ephemeris_path") config.dataset.celestialEphemerisPath = value;
    else if (key == "dataset.celestial.catalog_hash") config.dataset.celestialCatalogHash = value;
    else if (key == "dataset.celestial.ephemeris_hash") config.dataset.celestialEphemerisHash = value;
    else if (key == "adapter.id") config.adapter.id = toLower(trim(value));
    else if (key == "adapter.version") config.adapter.version = trim(value);
    else if (key == "adapter.manifest_path") config.adapter.manifestPath = trim(value);
    else if (key == "adapter.allowlist_path") config.adapter.allowlistPath = trim(value);
    else if (key == "adapter.core_version") config.adapter.coreVersion = trim(value);
    else if (key == "adapter.tools_version") config.adapter.toolsVersion = trim(value);
    else if (key == "adapter.ui_version") config.adapter.uiVersion = trim(value);
    else if (key == "adapter.contract_version") config.adapter.adapterContractVersion = trim(value);
    else if (key == "ui.contract_version") config.adapter.uiContractVersion = trim(value);
    else if (key == "adapter.allowlist_max_age_days" && toInt(value, ival)) config.adapter.allowlistMaxAgeDays = ival;
    else if (key == "ui.surface" && toUiSurface(value, uiSurface)) config.adapter.uiSurface = uiSurface;
    else if (key == "ui.surface") setIssue(result, key, "invalid ui surface");
    else if (key == "plugin.id") config.plugin.id = toLower(trim(value));
    else if (key == "plugin.version") config.plugin.version = trim(value);
    else if (key == "plugin.signature_hash") config.plugin.signatureHash = trim(value);
    else if (key == "plugin.signature_algorithm") config.plugin.signatureAlgorithm = toLower(trim(value));
    else if (key == "plugin.allowlist.id") config.plugin.allowlistId = toLower(trim(value));
    else if (key == "plugin.allowlist.version") config.plugin.allowlistVersion = trim(value);
    else if (key == "plugin.allowlist.signature_hash") config.plugin.allowlistSignatureHash = trim(value);
    else if (key == "plugin.allowlist.signature_algorithm") config.plugin.allowlistSignatureAlgorithm = toLower(trim(value));
    else if (key == "plugin.authorization_required" && toBool(value, bval)) config.plugin.authorizationRequired = bval;
    else if (key == "plugin.authorization_granted" && toBool(value, bval)) config.plugin.authorizationGranted = bval;
    else if (key == "plugin.device_driver" && toBool(value, bval)) config.plugin.deviceDriver = bval;
    else if (key == "mode.ladder_order") config.mode.ladderOrder = splitList(value);
    else if (key == "mode.min_healthy_count" && toInt(value, ival)) config.mode.minHealthyCount = ival;
    else if (key == "mode.min_dwell_steps" && toInt(value, ival)) config.mode.minDwellSteps = ival;
    else if (key == "mode.max_stale_count" && toInt(value, ival)) config.mode.maxStaleCount = ival;
    else if (key == "mode.max_low_confidence_count" && toInt(value, ival)) config.mode.maxLowConfidenceCount = ival;
    else if (key == "mode.lockout_steps" && toInt(value, ival)) config.mode.lockoutSteps = ival;
    else if (key == "mode.history_window" && toInt(value, ival)) config.mode.historyWindow = ival;
    else if (key == "fusion.max_data_age_seconds" && toDouble(value, dval)) config.fusion.maxDataAgeSeconds = dval;
    else if (key == "fusion.disagreement_threshold" && toDouble(value, dval)) config.fusion.disagreementThreshold = dval;
    else if (key == "fusion.min_confidence" && toDouble(value, dval)) config.fusion.minConfidence = dval;
    else if (key == "fusion.max_disagreement_count" && toInt(value, ival)) config.fusion.maxDisagreementCount = ival;
    else if (key == "fusion.max_residual_age_seconds" && toDouble(value, dval)) config.fusion.maxResidualAgeSeconds = dval;
    else if (key == "scheduler.primary_budget_ms" && toDouble(value, dval)) config.scheduler.primaryBudgetMs = dval;
    else if (key == "scheduler.aux_budget_ms" && toDouble(value, dval)) config.scheduler.auxBudgetMs = dval;
    else if (key == "scheduler.max_aux_pipelines" && toInt(value, ival))
    {
        if (ival < 0)
        {
            setIssue(result, key, "must be >= 0");
        }
        else
        {
            config.scheduler.maxAuxPipelines = static_cast<std::size_t>(ival);
        }
    }
    else if (key == "scheduler.aux_min_service_interval" && toDouble(value, dval)) config.scheduler.auxMinServiceIntervalSeconds = dval;
    else if (key == "scheduler.allow_snapshot_overlap" && toBool(value, bval)) config.scheduler.allowSnapshotOverlap = bval;
    else if (key.rfind("fusion.source_weights.", 0) == 0)
    {
        std::string sensor = toLower(trim(key.substr(std::string("fusion.source_weights.").size())));
        if (sensor.empty())
        {
            setIssue(result, key, "missing sensor name");
        }
        else if (toDouble(value, dval))
        {
            config.fusion.sourceWeights[sensor] = dval;
        }
        else
        {
            setIssue(result, key, "invalid weight");
        }
    }
    else
    {
        setIssue(result, key, "unknown or invalid value");
    }
}

void validateRange(ConfigResult &result, const std::string &key, double value, double minValue, double maxValue,
                   bool minInclusive = true, bool maxInclusive = true)
{
    bool below = minInclusive ? (value < minValue) : (value <= minValue);
    bool above = maxInclusive ? (value > maxValue) : (value >= maxValue);
    if (below || above)
    {
        setIssue(result, key, "out of range");
    }
}

void validateConfig(ConfigResult &result)
{
    const SimConfig &config = result.config;

    if (config.version != kExpectedVersion)
    {
        setIssue(result, "config.version", "unsupported version");
    }

    validateRange(result, "state.position.x", config.initialState.position.x, -1e6, 1e6);
    validateRange(result, "state.position.y", config.initialState.position.y, -1e6, 1e6);
    validateRange(result, "state.position.z", config.initialState.position.z, -1e6, 1e6);
    validateRange(result, "state.velocity.x", config.initialState.velocity.x, -1e4, 1e4);
    validateRange(result, "state.velocity.y", config.initialState.velocity.y, -1e4, 1e4);
    validateRange(result, "state.velocity.z", config.initialState.velocity.z, -1e4, 1e4);
    validateRange(result, "state.acceleration.x", config.initialState.acceleration.x, -1e3, 1e3);
    validateRange(result, "state.acceleration.y", config.initialState.acceleration.y, -1e3, 1e3);
    validateRange(result, "state.acceleration.z", config.initialState.acceleration.z, -1e3, 1e3);
    validateRange(result, "state.time", config.initialState.time, 0.0, 1e6);

    validateRange(result, "sim.dt", config.dt, 0.0, 10.0, false, true);
    validateRange(result, "sim.steps", static_cast<double>(config.steps), 1.0, 1e7);
    validateRange(result, "sim.seed", static_cast<double>(config.seed), 0.0, 4294967295.0);

    validateRange(result, "bounds.min.x", config.bounds.minPosition.x, -1e6, 1e6);
    validateRange(result, "bounds.min.y", config.bounds.minPosition.y, -1e6, 1e6);
    validateRange(result, "bounds.min.z", config.bounds.minPosition.z, -1e6, 1e6);
    validateRange(result, "bounds.max.x", config.bounds.maxPosition.x, -1e6, 1e6);
    validateRange(result, "bounds.max.y", config.bounds.maxPosition.y, -1e6, 1e6);
    validateRange(result, "bounds.max.z", config.bounds.maxPosition.z, -1e6, 1e6);
    validateRange(result, "bounds.max_speed", config.bounds.maxSpeed, 0.0, 1e5, false, true);
    validateRange(result, "bounds.max_accel", config.bounds.maxAcceleration, 0.0, 1e4, false, true);
    validateRange(result, "bounds.max_turn_rate_deg", config.bounds.maxTurnRateDeg, 0.0, 360.0, false, true);

    if (config.bounds.minPosition.x > config.bounds.maxPosition.x ||
        config.bounds.minPosition.y > config.bounds.maxPosition.y ||
        config.bounds.minPosition.z > config.bounds.maxPosition.z)
    {
        setIssue(result, "bounds", "min greater than max");
    }

    validateRange(result, "maneuver.random_accel_std", config.maneuvers.randomAccelStd, 0.0, 1e3);
    validateRange(result, "maneuver.probability", config.maneuvers.maneuverProbability, 0.0, 1.0);

    auto validateSensor = [&](const SensorConfig &sensor, const std::string &name, bool rangeRequired)
    {
        validateRange(result, "sensor." + name + ".rate_hz", sensor.rateHz, 0.0, 2000.0, false, true);
        validateRange(result, "sensor." + name + ".noise_std", sensor.noiseStd, 0.0, 1e4);
        validateRange(result, "sensor." + name + ".dropout", sensor.dropoutProbability, 0.0, 1.0);
        validateRange(result, "sensor." + name + ".false_positive", sensor.falsePositiveProbability, 0.0, 1.0);
        if (rangeRequired)
        {
            validateRange(result, "sensor." + name + ".max_range", sensor.maxRange, 0.0, 1e7, false, true);
        }
    };

    validateSensor(config.gps, "gps", true);
    validateSensor(config.thermal, "thermal", true);
    validateSensor(config.deadReckoning, "dead_reckoning", true);
    validateSensor(config.imu, "imu", true);
    validateSensor(config.radar, "radar", true);
    validateSensor(config.vision, "vision", true);
    validateSensor(config.lidar, "lidar", true);
    validateSensor(config.magnetometer, "magnetometer", false);
    validateSensor(config.baro, "baro", true);
    validateSensor(config.celestial, "celestial", false);

    if (config.policy.overrideTimeoutSeconds < 0)
    {
        setIssue(result, "policy.network_aid.override_timeout_seconds", "must be >= 0");
    }
    if (config.adapter.allowlistMaxAgeDays < 0 || config.adapter.allowlistMaxAgeDays > 3650)
    {
        setIssue(result, "adapter.allowlist_max_age_days", "must be between 0 and 3650");
    }

    validateRange(result, "mode.min_healthy_count", static_cast<double>(config.mode.minHealthyCount), 1.0, 1000.0);
    validateRange(result, "mode.min_dwell_steps", static_cast<double>(config.mode.minDwellSteps), 0.0, 100000.0);
    validateRange(result, "mode.max_stale_count", static_cast<double>(config.mode.maxStaleCount), 0.0, 100000.0);
    validateRange(result, "mode.max_low_confidence_count", static_cast<double>(config.mode.maxLowConfidenceCount), 0.0, 100000.0);
    validateRange(result, "mode.lockout_steps", static_cast<double>(config.mode.lockoutSteps), 0.0, 100000.0);
    validateRange(result, "mode.history_window", static_cast<double>(config.mode.historyWindow), 0.0, 100000.0);

    for (const auto &modeName : config.mode.ladderOrder)
    {
        if (!isValidModeName(modeName))
        {
            setIssue(result, "mode.ladder_order", "invalid mode in ladder_order: " + modeName);
            break;
        }
    }

    validateRange(result, "fusion.max_data_age_seconds", config.fusion.maxDataAgeSeconds, 0.0, 60.0, false, true);
    validateRange(result, "fusion.disagreement_threshold", config.fusion.disagreementThreshold, 0.0, 1e6, false, true);
    validateRange(result, "fusion.min_confidence", config.fusion.minConfidence, 0.0, 1.0);
    validateRange(result, "fusion.max_disagreement_count", static_cast<double>(config.fusion.maxDisagreementCount), 0.0, 100000.0);
    validateRange(result, "fusion.max_residual_age_seconds", config.fusion.maxResidualAgeSeconds, 0.0, 60.0, false, true);

    for (const auto &entry : config.fusion.sourceWeights)
    {
        if (entry.first.empty())
        {
            setIssue(result, "fusion.source_weights", "sensor name missing");
        }
        validateRange(result, "fusion.source_weights." + entry.first, entry.second, 0.0, 1.0);
    }

    validateRange(result, "scheduler.primary_budget_ms", config.scheduler.primaryBudgetMs, 0.0, 1000.0);
    validateRange(result, "scheduler.aux_budget_ms", config.scheduler.auxBudgetMs, 0.0, 1000.0);
    validateRange(result, "scheduler.max_aux_pipelines", static_cast<double>(config.scheduler.maxAuxPipelines), 0.0, 64.0);
    validateRange(result, "scheduler.aux_min_service_interval", config.scheduler.auxMinServiceIntervalSeconds, 0.0, 60.0);

    if (config.policy.roles.empty())
    {
        setIssue(result, "policy.roles", "must include at least one role");
    }

    if (config.hasParentProfile && config.parentProfile == config.platformProfile)
    {
        setIssue(result, "platform.profile_parent", "must not match platform.profile");
    }

    std::unordered_map<std::string, int> moduleCounts;
    for (const auto &module : config.childModules)
    {
        if (!isValidModuleName(module))
        {
            setIssue(result, "platform.child_modules", "invalid module identifier");
            break;
        }
        moduleCounts[module]++;
    }
    for (const auto &entry : moduleCounts)
    {
        if (entry.second > 1)
        {
            setIssue(result, "platform.child_modules", "duplicate module identifier");
            break;
        }
    }

    bool activeFound = false;
    for (const auto &role : config.policy.roles)
    {
        if (role == config.policy.activeRole)
        {
            activeFound = true;
            break;
        }
    }
    if (!activeFound)
    {
        setIssue(result, "policy.active_role", "role not defined");
    }

    for (const auto &entry : config.policy.rolePermissions)
    {
        const std::string &role = entry.first;
        bool found = false;
        for (const auto &defined : config.policy.roles)
        {
            if (defined == role)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            setIssue(result, "policy.role_permissions." + role, "role not defined");
        }
    }

    if (config.policy.authorization.configured())
    {
        if (config.policy.authorization.version.empty())
        {
            setIssue(result, "policy.authorization.version", "required when authorization bundle is configured");
        }
        if (config.policy.authorization.source.empty())
        {
            setIssue(result, "policy.authorization.source", "required when authorization bundle is configured");
        }
        if (config.policy.authorization.allowedModes.empty())
        {
            setIssue(result, "policy.authorization.allowed_modes", "must include at least one mode");
        }
        for (const auto &mode : config.policy.authorization.allowedModes)
        {
            if (!isValidModeName(mode))
            {
                setIssue(result, "policy.authorization.allowed_modes", "invalid mode in authorization bundle: " + mode);
                break;
            }
        }
    }

    if (config.provenance.allowedInputs.empty())
    {
        setIssue(result, "provenance.allowed_inputs", "must include at least one provenance value");
    }
    else
    {
        std::unordered_map<int, int> counts;
        for (const auto &mode : config.provenance.allowedInputs)
        {
            counts[static_cast<int>(mode)]++;
        }
        for (const auto &entry : counts)
        {
            if (entry.second > 1)
            {
                setIssue(result, "provenance.allowed_inputs", "duplicate provenance entries");
                break;
            }
        }
    }

    bool runModeAllowed = false;
    for (const auto &mode : config.provenance.allowedInputs)
    {
        if (mode == config.provenance.runMode)
        {
            runModeAllowed = true;
            break;
        }
    }
    if (!runModeAllowed)
    {
        setIssue(result, "provenance.run_mode", "run_mode must be included in allowed_inputs");
    }

    if (!config.provenance.allowMixed && config.provenance.allowedInputs.size() > 1)
    {
        setIssue(result, "provenance.allow_mixed", "mixed provenance not allowed with allow_mixed=false");
    }

    if (config.dataset.maxSizeMB < 0.0)
    {
        setIssue(result, "dataset.celestial.max_size_mb", "must be >= 0");
    }

    if (!config.dataset.celestialCatalogPath.empty() && config.dataset.celestialCatalogHash.empty())
    {
        setIssue(result, "dataset.celestial.catalog_hash", "required when catalog_path is set");
    }
    if (!config.dataset.celestialEphemerisPath.empty() && config.dataset.celestialEphemerisHash.empty())
    {
        setIssue(result, "dataset.celestial.ephemeris_hash", "required when ephemeris_path is set");
    }

    if (!config.adapter.id.empty())
    {
        if (!isValidAdapterId(config.adapter.id))
        {
            setIssue(result, "adapter.id", "invalid adapter identifier");
        }
        if (config.adapter.version.empty())
        {
            setIssue(result, "adapter.version", "required when adapter.id is set");
        }
        if (config.adapter.manifestPath.empty() && !isOfficialAdapterId(config.adapter.id))
        {
            setIssue(result, "adapter.manifest_path", "required for non-official adapter");
        }
    }
    if (config.adapter.id.empty())
    {
        if (!config.adapter.manifestPath.empty())
        {
            setIssue(result, "adapter.manifest_path", "adapter.id required for manifest_path");
        }
        if (!config.adapter.allowlistPath.empty())
        {
            setIssue(result, "adapter.allowlist_path", "adapter.id required for allowlist_path");
        }
    }
    if (!isSemver(config.adapter.coreVersion))
    {
        setIssue(result, "adapter.core_version", "invalid semantic version");
    }
    if (!isSemver(config.adapter.toolsVersion))
    {
        setIssue(result, "adapter.tools_version", "invalid semantic version");
    }
    if (!isSemver(config.adapter.uiVersion))
    {
        setIssue(result, "adapter.ui_version", "invalid semantic version");
    }
    if (!isSemver(config.adapter.adapterContractVersion))
    {
        setIssue(result, "adapter.contract_version", "invalid semantic version");
    }
    if (!isSemver(config.adapter.uiContractVersion))
    {
        setIssue(result, "ui.contract_version", "invalid semantic version");
    }
    if (!config.adapter.uiSurface.empty())
    {
        std::string normalizedUiSurface = config.adapter.uiSurface;
        if (!toUiSurface(normalizedUiSurface, normalizedUiSurface))
        {
            setIssue(result, "ui.surface", "invalid ui surface");
        }
    }

    if (config.plugin.id.empty())
    {
        if (!config.plugin.version.empty())
        {
            setIssue(result, "plugin.version", "plugin.id required");
        }
        if (!config.plugin.signatureHash.empty())
        {
            setIssue(result, "plugin.signature_hash", "plugin.id required");
        }
        if (!config.plugin.signatureAlgorithm.empty())
        {
            setIssue(result, "plugin.signature_algorithm", "plugin.id required");
        }
        if (!config.plugin.allowlistId.empty())
        {
            setIssue(result, "plugin.allowlist.id", "plugin.id required");
        }
        if (!config.plugin.allowlistVersion.empty())
        {
            setIssue(result, "plugin.allowlist.version", "plugin.id required");
        }
        if (!config.plugin.allowlistSignatureHash.empty())
        {
            setIssue(result, "plugin.allowlist.signature_hash", "plugin.id required");
        }
        if (!config.plugin.allowlistSignatureAlgorithm.empty())
        {
            setIssue(result, "plugin.allowlist.signature_algorithm", "plugin.id required");
        }
    }
    else
    {
        if (!isValidModuleName(config.plugin.id))
        {
            setIssue(result, "plugin.id", "invalid plugin identifier");
        }
        if (!isSemver(config.plugin.version))
        {
            setIssue(result, "plugin.version", "invalid semantic version");
        }
        if (config.plugin.signatureHash.empty() || !isSha256Hex(config.plugin.signatureHash))
        {
            setIssue(result, "plugin.signature_hash", "invalid sha256 hash");
        }
        if (config.plugin.signatureAlgorithm != "sha256")
        {
            setIssue(result, "plugin.signature_algorithm", "must be sha256");
        }
        if (!isValidModuleName(config.plugin.allowlistId))
        {
            setIssue(result, "plugin.allowlist.id", "invalid plugin identifier");
        }
        if (!isSemver(config.plugin.allowlistVersion))
        {
            setIssue(result, "plugin.allowlist.version", "invalid semantic version");
        }
        if (config.plugin.allowlistSignatureHash.empty() || !isSha256Hex(config.plugin.allowlistSignatureHash))
        {
            setIssue(result, "plugin.allowlist.signature_hash", "invalid sha256 hash");
        }
        if (config.plugin.allowlistSignatureAlgorithm != "sha256")
        {
            setIssue(result, "plugin.allowlist.signature_algorithm", "must be sha256");
        }
    }
}
} // namespace

ConfigResult loadSimConfig(const std::string &path)
{
    ConfigResult result;
    result.config.initialState = {{0.0, 0.0, 100.0}, {15.0, 10.0, 0.0}, {0.2, -0.1, 0.0}, 0.0};

    std::ifstream file(path);
    if (!file)
    {
        setIssue(result, path, "unable to open config");
        return result;
    }

    std::string line;
    bool versionSeen = false;
    while (std::getline(file, line))
    {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#')
        {
            continue;
        }

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos)
        {
            setIssue(result, trimmed, "missing '='");
            continue;
        }

        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));
        if (key == "config.version")
        {
            versionSeen = true;
        }
        applyValue(result.config, result, key, value);
    }

    if (!versionSeen)
    {
        setIssue(result, "config.version", "missing required key");
    }

    if (result.config.permittedSensors.empty())
    {
        std::vector<std::string> sensors;
        if (result.config.hasParentProfile)
        {
            sensors = defaultSensorsForProfile(result.config.parentProfile);
            appendUnique(sensors, defaultSensorsForProfile(result.config.platformProfile));
        }
        else
        {
            sensors = defaultSensorsForProfile(result.config.platformProfile);
        }
        result.config.permittedSensors = std::move(sensors);
    }

    if (result.config.mode.ladderOrder.empty())
    {
        result.config.mode.ladderOrder = {
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
    }

    validateConfig(result);

    if (result.ok && !result.config.adapter.id.empty())
    {
        std::string reason;
        if (!tools::validateAdapterSelection(result.config, reason))
        {
            setIssue(result, "adapter.registry", reason);
        }
    }

    if (result.ok && !result.config.plugin.id.empty())
    {
        PluginAuthRequest request;
        request.identity.id = result.config.plugin.id;
        request.identity.version = result.config.plugin.version;
        request.identity.deviceDriver = result.config.plugin.deviceDriver;
        request.signature.hash = result.config.plugin.signatureHash;
        request.signature.algorithm = result.config.plugin.signatureAlgorithm;
        request.allowlist.id = result.config.plugin.allowlistId;
        request.allowlist.version = result.config.plugin.allowlistVersion;
        request.allowlist.signatureHash = result.config.plugin.allowlistSignatureHash;
        request.allowlist.signatureAlgorithm = result.config.plugin.allowlistSignatureAlgorithm;
        request.authorization.required = result.config.plugin.authorizationRequired;
        request.authorization.granted = result.config.plugin.authorizationGranted;

        PluginAuthResult pluginResult;
        if (!validatePluginActivation(request, pluginResult))
        {
            setIssue(result, "plugin.auth", pluginResult.reason);
        }
    }

    return result;
}
