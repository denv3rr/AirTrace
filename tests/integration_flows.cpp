#include "core/mode_manager.h"
#include "core/sim_config.h"
#include "core/sensors.h"
#include "ui/input_harness.h"
#include "ui/menu.h"
#include "ui/simulation.h"

#include <cassert>
#include <cstdlib>
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

    void setProvenance(ProvenanceTag tag)
    {
        provenance = tag;
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
    std::ofstream file(path.string(), std::ios::trunc);
    file << contents;
    return path;
}

bool hasSensor(const std::vector<std::string> &sensors, const std::string &name)
{
    for (const auto &entry : sensors)
    {
        if (entry == name)
        {
            return true;
        }
    }
    return false;
}

ProvenanceTag mapProvenance(SimConfig::ProvenanceMode mode)
{
    switch (mode)
    {
    case SimConfig::ProvenanceMode::Operational:
        return ProvenanceTag::Operational;
    case SimConfig::ProvenanceMode::Simulation:
        return ProvenanceTag::Simulation;
    case SimConfig::ProvenanceMode::Test:
        return ProvenanceTag::Test;
    default:
        return ProvenanceTag::Unknown;
    }
}

UnknownProvenanceAction mapUnknownAction(SimConfig::UnknownProvenanceAction action)
{
    switch (action)
    {
    case SimConfig::UnknownProvenanceAction::Hold:
        return UnknownProvenanceAction::Hold;
    case SimConfig::UnknownProvenanceAction::Deny:
    default:
        return UnknownProvenanceAction::Deny;
    }
}

ModeManagerConfig buildModeConfig(const SimConfig &config)
{
    ModeManagerConfig modeConfig;
    modeConfig.minHealthyCount = config.mode.minHealthyCount;
    modeConfig.minDwellSteps = config.mode.minDwellSteps;
    modeConfig.maxDataAgeSeconds = config.fusion.maxDataAgeSeconds;
    modeConfig.minConfidence = config.fusion.minConfidence;
    modeConfig.maxStaleCount = config.mode.maxStaleCount;
    modeConfig.maxLowConfidenceCount = config.mode.maxLowConfidenceCount;
    modeConfig.lockoutSteps = config.mode.lockoutSteps;
    modeConfig.maxDisagreementCount = config.fusion.maxDisagreementCount;
    modeConfig.disagreementThreshold = config.fusion.disagreementThreshold;
    modeConfig.historyWindow = config.mode.historyWindow;
    modeConfig.maxResidualAgeSeconds = config.fusion.maxResidualAgeSeconds;
    modeConfig.permittedSensors = config.permittedSensors;
    modeConfig.ladderOrder = config.mode.ladderOrder;
    modeConfig.authorizationRequired = (config.provenance.runMode == SimConfig::ProvenanceMode::Operational);
    modeConfig.authorizationVerified = config.policy.authorization.configured();
    modeConfig.authorizationAllowedModes = config.policy.authorization.allowedModes;
    modeConfig.allowedProvenances.clear();
    for (const auto &mode : config.provenance.allowedInputs)
    {
        modeConfig.allowedProvenances.push_back(mapProvenance(mode));
    }
    modeConfig.provenanceAllowMixed = config.provenance.allowMixed;
    modeConfig.provenanceUnknownAction = mapUnknownAction(config.provenance.unknownAction);
    modeConfig.celestialAllowed = hasSensor(config.permittedSensors, "celestial");
    modeConfig.celestialDatasetAvailable = config.dataset.celestialAvailable();
    return modeConfig;
}

void setEnvVar(const std::string &key, const std::string &value)
{
#if defined(_WIN32)
    _putenv((key + "=" + value).c_str());
#else
    setenv(key.c_str(), value.c_str(), 1);
#endif
}

void unsetEnvVar(const std::string &key)
{
#if defined(_WIN32)
    _putenv((key + "=").c_str());
#else
    unsetenv(key.c_str());
#endif
}
} // namespace

int main()
{
    std::filesystem::path inheritConfig = writeConfigFile(
        "airtrace_inherit.cfg",
        "config.version=1.0\n"
        "platform.profile=ground\n"
        "platform.profile_parent=space\n");
    ConfigResult inheritResult = loadSimConfig(inheritConfig.string());
    assert(inheritResult.ok);
    std::vector<std::string> expectedSensors = {
        "gps",
        "imu",
        "celestial",
        "dead_reckoning",
        "vision",
        "lidar",
        "radar",
        "thermal",
        "magnetometer",
        "baro"};
    assert(inheritResult.config.permittedSensors == expectedSensors);
    std::filesystem::remove(inheritConfig);

    std::filesystem::path invalidParentConfig = writeConfigFile(
        "airtrace_invalid_parent.cfg",
        "config.version=1.0\n"
        "platform.profile=air\n"
        "platform.profile_parent=air\n");
    ConfigResult invalidParentResult = loadSimConfig(invalidParentConfig.string());
    assert(!invalidParentResult.ok);
    std::filesystem::remove(invalidParentConfig);

    std::filesystem::path invalidModuleConfig = writeConfigFile(
        "airtrace_invalid_module.cfg",
        "config.version=1.0\n"
        "platform.child_modules=imu_stack,radar@front\n");
    ConfigResult invalidModuleResult = loadSimConfig(invalidModuleConfig.string());
    assert(!invalidModuleResult.ok);
    std::filesystem::remove(invalidModuleConfig);

    std::filesystem::path ladderConfig = writeConfigFile(
        "airtrace_ladder.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=simulation\n"
        "provenance.allowed_inputs=simulation\n"
        "platform.permitted_sensors=gps,radar\n"
        "mode.min_healthy_count=1\n"
        "mode.min_dwell_steps=0\n"
        "mode.ladder_order=gps,radar,hold\n");
    ConfigResult ladderResult = loadSimConfig(ladderConfig.string());
    assert(ladderResult.ok);
    ModeManager ladderManager(buildModeConfig(ladderResult.config));
    TestSensor ladderGps("gps");
    TestSensor ladderRadar("radar");
    ladderGps.setProvenance(ProvenanceTag::Simulation);
    ladderRadar.setProvenance(ProvenanceTag::Simulation);
    std::vector<SensorBase *> ladderSensors{&ladderGps, &ladderRadar};
    ladderGps.setStatus(false, 2.0, 0.2);
    ladderRadar.setStatus(true, 0.1, 0.9);
    ModeDecision ladderDecision = ladderManager.decide(ladderSensors);
    assert(ladderDecision.mode == TrackingMode::Radar);
    std::filesystem::remove(ladderConfig);

    std::filesystem::path datasetMissingConfig = writeConfigFile(
        "airtrace_dataset_missing.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=simulation\n"
        "provenance.allowed_inputs=simulation\n"
        "platform.permitted_sensors=celestial\n"
        "mode.min_healthy_count=1\n"
        "mode.min_dwell_steps=0\n"
        "mode.ladder_order=celestial,hold\n");
    ConfigResult datasetMissingResult = loadSimConfig(datasetMissingConfig.string());
    assert(datasetMissingResult.ok);
    ModeManager datasetMissingManager(buildModeConfig(datasetMissingResult.config));
    TestSensor datasetCelestial("celestial");
    datasetCelestial.setProvenance(ProvenanceTag::Simulation);
    std::vector<SensorBase *> datasetSensors{&datasetCelestial};
    datasetCelestial.setStatus(true, 0.1, 0.9);
    ModeDecision datasetMissingDecision = datasetMissingManager.decide(datasetSensors);
    assert(datasetMissingDecision.mode == TrackingMode::Hold);
    std::filesystem::remove(datasetMissingConfig);

    std::filesystem::path datasetConfig = writeConfigFile(
        "airtrace_dataset_ok.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=simulation\n"
        "provenance.allowed_inputs=simulation\n"
        "platform.permitted_sensors=celestial\n"
        "mode.min_healthy_count=1\n"
        "mode.min_dwell_steps=0\n"
        "mode.ladder_order=celestial,hold\n"
        "dataset.celestial.catalog_path=data/catalog\n"
        "dataset.celestial.catalog_hash=abc\n"
        "dataset.celestial.ephemeris_path=data/ephemeris\n"
        "dataset.celestial.ephemeris_hash=def\n");
    ConfigResult datasetResult = loadSimConfig(datasetConfig.string());
    assert(datasetResult.ok);
    ModeManager datasetManager(buildModeConfig(datasetResult.config));
    datasetCelestial.setProvenance(ProvenanceTag::Simulation);
    datasetCelestial.setStatus(true, 0.1, 0.9);
    ModeDecision datasetDecision = datasetManager.decide(datasetSensors);
    assert(datasetDecision.mode == TrackingMode::Celestial);
    std::filesystem::remove(datasetConfig);

    std::filesystem::path uiConfig = writeConfigFile(
        "airtrace_ui.cfg",
        "config.version=1.0\n");
    std::filesystem::path harnessCommands = writeConfigFile(
        "airtrace_ui_harness.txt",
        "select|AirTrace - Main Menu|1\n"
        "select|AirTrace - Test Menu|2\n"
        "select|AirTrace - Main Menu|4\n");

    std::filesystem::path profileConfig = writeConfigFile(
        "airtrace_ui_profile.cfg",
        "config.version=1.0\n"
        "platform.profile=ground\n"
        "platform.profile_parent=space\n"
        "platform.child_modules=imu_stack,radar_frontend\n");

    setEnvVar("AIRTRACE_TEST_HARNESS", "1");
    setEnvVar("AIRTRACE_HARNESS_COMMANDS", harnessCommands.string());
    ui::InputHarness harness(harnessCommands.string());
    ui::setInputHarness(&harness);

    bool uiConfigLoaded = initializeUiContext(profileConfig.string());
    assert(uiConfigLoaded);
    const UiStatus &status = getUiStatus();
    assert(status.parentProfile == "space");
    assert(status.childModules == "imu_stack,radar_frontend");
    bool menuResult = showMainMenu();
    assert(menuResult);

    ui::setInputHarness(nullptr);
    unsetEnvVar("AIRTRACE_TEST_HARNESS");
    unsetEnvVar("AIRTRACE_HARNESS_COMMANDS");
    std::filesystem::remove(harnessCommands);
    std::filesystem::remove(profileConfig);

    std::filesystem::path testHarnessCommands = writeConfigFile(
        "airtrace_testmode_harness.txt",
        "input_int|Enter movement speed (1-100, sim steps/sec): |10\n"
        "input_int|Enter number of iterations (0 for infinite): |0\n"
        "select|Testing and Debugging Menu|99\n");

    setEnvVar("AIRTRACE_TEST_HARNESS", "1");
    setEnvVar("AIRTRACE_HARNESS_COMMANDS", testHarnessCommands.string());
    ui::InputHarness testHarness(testHarnessCommands.string());
    ui::setInputHarness(&testHarness);

    uiConfigLoaded = initializeUiContext(uiConfig.string());
    assert(uiConfigLoaded);
    runTestMode();
    assert(getUiStatus().denialReason == "menu_selection_invalid");

    ui::setInputHarness(nullptr);
    unsetEnvVar("AIRTRACE_TEST_HARNESS");
    unsetEnvVar("AIRTRACE_HARNESS_COMMANDS");
    std::filesystem::remove(testHarnessCommands);
    std::filesystem::remove(uiConfig);

    return 0;
}
