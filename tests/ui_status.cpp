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
        "provenance.unknown_action=deny\n"
        "front_view.enabled=true\n"
        "front_view.spoof.enabled=true\n"
        "front_view.display_families=eo_gray,ir_white_hot,proximity_2d\n"
        "front_view.frame.max_age_ms=500\n"
        "front_view.frame.min_confidence=0.3\n"
        "front_view.multi_view.max_streams=2\n"
        "front_view.multi_view.stream_ids=primary,turret\n"
        "front_view.stabilization.enabled=true\n"
        "front_view.stabilization.mode=gimbal_lock\n"
        "front_view.gimbal.enabled=true\n"
        "front_view.gimbal.max_yaw_rate_deg_s=220\n"
        "front_view.gimbal.max_pitch_rate_deg_s=160\n"
        "front_view.threading.enabled=true\n"
        "front_view.threading.max_workers=2\n");
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
    assert(status.adapterStatus == "none");
    assert(status.adapterReason == "none");
    assert(status.adapterContext.empty());

    const std::vector<std::string> profiles = uiListPlatformProfiles();
    assert(profiles.size() == 8);
    assert(profiles[0] == "base");
    assert(profiles[1] == "air");
    assert(profiles[7] == "subsea");

    PlatformSuiteResult invalidProfile = uiRunPlatformSuite("invalid_profile");
    assert(!invalidProfile.pass);
    assert(invalidProfile.reason == "platform_profile_invalid");

    PlatformSuiteResult airSuite = uiRunPlatformSuite("air");
    assert(airSuite.pass);
    assert(airSuite.profile == "air");
    assert(airSuite.sensorsValidated);
    assert(airSuite.adapterValidated);
    assert(airSuite.modeOutputValidated);

    const std::vector<PlatformSuiteResult> allSuites = uiRunAllPlatformSuites();
    assert(allSuites.size() == profiles.size());
    bool foundAir = false;
    for (const auto &entry : allSuites)
    {
        if (entry.profile == "air")
        {
            foundAir = true;
        }
    }
    assert(foundAir);

    const ExternalIoEnvelope envelope = uiBuildExternalIoEnvelope();
    assert(envelope.metadata.schemaVersion == "1.0.0");
    assert(envelope.metadata.platformProfile == getUiStatus().platformProfile);
    assert(!envelope.mode.activeMode.empty());
    assert(!envelope.sensors.empty());
    const std::string envelopeJson = uiBuildExternalIoEnvelopeJson();
    assert(envelopeJson.find("\"schema_version\":\"1.0.0\"") != std::string::npos);
    assert(envelopeJson.find("\"platform_profile\":\"" + getUiStatus().platformProfile + "\"") != std::string::npos);
    assert(envelopeJson.find("\"sensors\"") != std::string::npos);

    const std::vector<std::string> frontViewModes = uiListFrontViewDisplayModes();
    assert(!frontViewModes.empty());
    std::string frontViewReason;
    bool frontViewOk = uiRunFrontViewDisplaySuite(true, frontViewReason);
    assert(frontViewOk);
    assert(frontViewReason == "ok" || frontViewReason == "render_latency_exceeded");
    const UiStatus &frontViewStatus = getUiStatus();
    assert(!frontViewStatus.frontViewMode.empty());
    assert(frontViewStatus.frontViewSpoofActive);
    assert(frontViewStatus.frontViewTimestampMs <= 1000000ULL);
    assert(frontViewStatus.frontViewFrameAgeMs >= 0.0);
    assert(!frontViewStatus.frontViewSourceId.empty());
    assert(!frontViewStatus.frontViewStreamId.empty());
    assert(frontViewStatus.frontViewStreamCount >= 1);
    assert(frontViewStatus.frontViewMaxConcurrentViews == 2U);
    assert(!frontViewStatus.frontViewStreams.empty());
    const ExternalIoEnvelope frontViewEnvelope = uiBuildExternalIoEnvelope();
    assert(!frontViewEnvelope.frontView.activeMode.empty());
    assert(frontViewEnvelope.frontView.spoofActive);
    assert(!frontViewEnvelope.frontView.sourceId.empty());
    assert(frontViewEnvelope.frontView.timestampMs <= 1000000ULL);
    assert(!frontViewEnvelope.frontView.streamId.empty());
    assert(frontViewEnvelope.frontView.streamCount >= 1);
    assert(frontViewEnvelope.frontView.maxConcurrentViews == 2U);
    assert(!frontViewEnvelope.frontViewStreams.empty());
    const std::string frontViewJson = uiBuildExternalIoEnvelopeJson();
    assert(frontViewJson.find("\"front_view\"") != std::string::npos);
    assert(frontViewJson.find("\"active_mode\":\"" + frontViewEnvelope.frontView.activeMode + "\"") != std::string::npos);
    assert(frontViewJson.find("\"source_id\":\"" + frontViewEnvelope.frontView.sourceId + "\"") != std::string::npos);
    assert(frontViewJson.find("\"front_view_streams\"") != std::string::npos);

    std::filesystem::remove(configPath);
}
