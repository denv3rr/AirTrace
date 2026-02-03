#include "core/multi_modal_types.h"
#include "ui/simulation.h"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace
{
std::filesystem::path writeConfigFile(const std::string &name, const std::string &contents)
{
    std::filesystem::path path = std::filesystem::current_path() / name;
    std::ofstream file(path.string(), std::ios::trunc);
    file << contents;
    return path;
}

ModeDecisionDetail sampleDetail()
{
    ModeDecisionDetail detail;
    detail.selectedMode = "gps_ins";
    detail.contributors = {"gps", "imu"};
    detail.confidence = 0.75;
    detail.reason = "gps_ins_eligible";
    detail.downgradeReason = "residual_conflict";
    detail.disqualifiedSources.push_back({"vio", "vision", "stale"});
    detail.disqualifiedSources.push_back({"vio", "imu", "no_measurement"});
    detail.lockouts.push_back({"gps", 2, "stale"});
    return detail;
}

std::vector<SensorUiSnapshot> sampleSensors()
{
    std::vector<SensorUiSnapshot> sensors;
    SensorUiSnapshot gps;
    gps.name = "gps";
    gps.available = true;
    gps.healthy = true;
    gps.hasMeasurement = true;
    gps.timeSinceLastValid = 0.1;
    gps.confidence = 0.9;
    sensors.push_back(gps);

    SensorUiSnapshot imu;
    imu.name = "imu";
    imu.available = true;
    imu.healthy = true;
    imu.hasMeasurement = true;
    imu.timeSinceLastValid = 0.2;
    imu.confidence = 0.8;
    sensors.push_back(imu);

    SensorUiSnapshot vision;
    vision.name = "vision";
    vision.available = true;
    vision.healthy = false;
    vision.hasMeasurement = false;
    vision.timeSinceLastValid = 1.5;
    vision.confidence = 0.1;
    vision.lastError = "stale";
    sensors.push_back(vision);

    return sensors;
}
} // namespace

void runStatusTests()
{
    std::filesystem::path configPath = writeConfigFile(
        "airtrace_ui_status.cfg",
        "config.version=1.0\n"
        "provenance.run_mode=simulation\n"
        "provenance.allowed_inputs=simulation\n"
        "provenance.allow_mixed=false\n"
        "provenance.unknown_action=deny\n");
    bool loaded = initializeUiContext(configPath.string());
    assert(loaded);
    ModeDecisionDetail detail = sampleDetail();
    std::vector<SensorUiSnapshot> sensors = sampleSensors();
    updateUiFromModeDecision(detail, sensors);

    const UiStatus &status = getUiStatus();
    assert(status.activeSource == "gps_ins");
    assert(status.contributors == "gps,imu");
    assert(status.modeConfidence == 0.75);
    assert(status.decisionReason == "gps_ins_eligible");
    assert(status.denialReason == "residual_conflict");
    assert(status.disqualifiedSources.find("vio:vision=stale") != std::string::npos);
    assert(status.disqualifiedSources.find("vio:imu=no_measurement") != std::string::npos);
    assert(status.lockoutStatus.find("gps(steps=2,reason=stale)") != std::string::npos);
    assert(status.ladderStatus.find("gps_ins:selected") != std::string::npos);
    assert(status.ladderStatus.find("vio:disq(vision=stale,imu=no_measurement)") != std::string::npos);
    assert(status.provenanceStatus.find("run=simulation") != std::string::npos);
    assert(status.sensorStatusSummary.find("gps[avail=y,health=y,meas=y") != std::string::npos);
    assert(status.sensorStatusSummary.find("lockout=steps:2,reason:stale") != std::string::npos);
    assert(status.sensorStatusSummary.find("vision[avail=y,health=n,meas=n") != std::string::npos);

    std::filesystem::remove(configPath);
}
