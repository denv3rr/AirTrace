// This file holds test functionality for different modes

#include "ui/simulation.h"
#include "core/Tracker.h"
#include "ui/inputValidation.h"
#include "ui/scenario.h"
#include "ui/tui.h"
#include "ui/alerts.h"
#include "ui/adapter_ui_mapping.h"
#include "ui/front_view.h"
#include "tools/audit_log.h"
#include "tools/adapter_registry_loader.h"
#include "tools/sim_config_loader.h"
#include <iostream>
#include <cmath>
#include <atomic>
#include <iomanip>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <cctype>
#include <algorithm>
#include <set>
#include <unordered_map>

// Global vector to store simulation history
std::vector<SimulationData> simulationHistory;

// Atomic variable to allow exit during test mode
std::atomic<bool> exitRequested(false);
std::atomic<bool> exitRequestedByUser(false);

namespace
{
struct UiContext
{
    SimConfig config{};
    bool configLoaded = false;
    unsigned int seed = 42;
    std::mt19937 rng{seed};
    UiStatus status{};
    std::vector<SensorUiSnapshot> lastSensors{};
};

UiContext uiContext{};

bool hasPermission(const std::string &permission)
{
#if defined(AIRTRACE_TEST_HARNESS)
    if (permission == "test_mode" || permission == "simulation_history" || permission == "simulation_delete" ||
        permission == "platform_workbench" || permission == "front_view_workbench")
    {
        return true;
    }
#endif
    const auto &policy = uiContext.config.policy;
    auto it = policy.rolePermissions.find(policy.activeRole);
    if (it == policy.rolePermissions.end())
    {
        return false;
    }
    for (const auto &entry : it->second)
    {
        if (entry == "all" || entry == permission)
        {
            return true;
        }
    }
    return false;
}

bool ensureAuditHealthy(const std::string &context)
{
    setUiLoggingStatus(tools::auditLogStatus());
    if (tools::auditLogHealthy())
    {
        return true;
    }
#if defined(AIRTRACE_TEST_HARNESS)
    tools::AuditLogConfig testConfig{};
    testConfig.logPath = "audit_log_test.jsonl";
    testConfig.configPath = "configs/sim_default.cfg";
    testConfig.buildId = "test";
    testConfig.role = "test";
    if (uiContext.configLoaded)
    {
        testConfig.configVersion = uiContext.config.version;
        testConfig.seed = uiContext.seed;
    }
    std::string status;
    if (tools::initializeAuditLog(testConfig, status))
    {
        if (uiContext.configLoaded)
        {
            tools::setAuditRunContext("", uiContext.config.version, uiContext.seed);
        }
        setUiLoggingStatus(tools::auditLogStatus());
        tools::logAuditEvent("audit_recovered", "audit log initialized for test harness", context);
        return true;
    }
#endif
    setUiDenialReason("audit_unavailable");
    std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
    tools::logAuditEvent("audit_unavailable", "audit log unavailable", context);
    return false;
}

bool requirePermission(const std::string &permission, const std::string &context)
{
    if (hasPermission(permission))
    {
        return true;
    }
    setUiDenialReason("sim_not_authorized");
    std::cerr << "Not authorized for " << permission << ". Recovery: update role permissions and retry.\n";
    tools::logAuditEvent("authorization_denied", "permission denied", context);
    return false;
}

bool logAuditOrDeny(const std::string &eventType, const std::string &message, const std::string &detail, const std::string &context)
{
    if (tools::logAuditEvent(eventType, message, detail))
    {
        return true;
    }
    setUiLoggingStatus(tools::auditLogStatus());
    setUiDenialReason("audit_unavailable");
    std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
    std::cerr << "Context: " << context << "\n";
    return false;
}

void renderStatusBanner(const std::string &context)
{
    const UiStatus &status = getUiStatus();
    std::string adapterLabel = "none";
    if (!status.adapterId.empty())
    {
        adapterLabel = status.adapterId;
        if (!status.adapterVersion.empty())
        {
            adapterLabel += "@";
            adapterLabel += status.adapterVersion;
        }
        else
        {
            adapterLabel += "@unknown";
        }
    }
    std::cout << "\n[STATUS] context=" << context
              << " profile=" << status.platformProfile
              << " parent=" << (status.parentProfile.empty() ? "none" : status.parentProfile)
              << " modules=" << (status.childModules.empty() ? "none" : status.childModules)
              << " source=" << (status.activeSource.empty() ? "none" : status.activeSource)
              << " contributors=" << (status.contributors.empty() ? "none" : status.contributors)
              << " conf=" << status.modeConfidence
              << " disq=" << (status.disqualifiedSources.empty() ? "none" : status.disqualifiedSources)
              << " lockout=" << (status.lockoutStatus.empty() ? "none" : status.lockoutStatus)
              << " ladder=" << (status.ladderStatus.empty() ? "none" : status.ladderStatus)
              << " conc=" << (status.concurrencyStatus.empty() ? "none" : status.concurrencyStatus)
              << " decision=" << (status.decisionReason.empty() ? "none" : status.decisionReason)
              << " denial=" << (status.denialReason.empty() ? "none" : status.denialReason)
              << " auth=" << (status.authStatus.empty() ? "unknown" : status.authStatus)
              << " prov=" << (status.provenanceStatus.empty() ? "unknown" : status.provenanceStatus)
              << " adapter=" << adapterLabel
              << " surface=" << (status.adapterSurface.empty() ? "tui" : status.adapterSurface)
              << " adapter_status=" << (status.adapterStatus.empty() ? "unknown" : status.adapterStatus)
              << " adapter_reason=" << (status.adapterReason.empty() ? "none" : status.adapterReason)
              << " front_view_mode=" << (status.frontViewMode.empty() ? "none" : status.frontViewMode)
              << " front_view_stream=" << (status.frontViewStreamId.empty() ? "none" : status.frontViewStreamId)
              << " front_view_seq=" << status.frontViewSequence
              << " front_view_ts_ms=" << status.frontViewTimestampMs
              << " front_view_latency_ms=" << status.frontViewLatencyMs
              << " log=" << (status.loggingStatus.empty() ? "unknown" : status.loggingStatus)
              << " sensors=" << (status.sensorStatusSummary.empty() ? "none" : status.sensorStatusSummary)
              << " seed=" << status.seed
              << " det=" << (status.deterministic ? "on" : "off")
              << "\n";
    if (!status.ladderStatus.empty())
    {
        std::cout << "LADDER: " << status.ladderStatus << "\n";
    }
    if (!status.sensorStatusSummary.empty())
    {
        std::cout << "SENSORS: " << status.sensorStatusSummary << "\n";
    }
    if (!status.adapterFields.empty())
    {
        std::cout << "ADAPTER FIELDS: " << status.adapterFields << "\n";
    }
    if (!status.adapterApproval.empty())
    {
        std::cout << "ADAPTER APPROVAL: " << status.adapterApproval << "\n";
    }
    if (!status.adapterContext.empty())
    {
        std::cout << "ADAPTER CONTEXT: " << status.adapterContext << "\n";
    }
    if (!status.frontViewMode.empty() && status.frontViewMode != "none")
    {
        std::cout << "FRONT VIEW: mode=" << status.frontViewMode
                  << " state=" << (status.frontViewViewState.empty() ? "none" : status.frontViewViewState)
                  << " frame=" << (status.frontViewFrameId.empty() ? "none" : status.frontViewFrameId)
                  << " source=" << (status.frontViewSourceId.empty() ? "none" : status.frontViewSourceId)
                  << " sensor=" << (status.frontViewSensorType.empty() ? "none" : status.frontViewSensorType)
                  << " stream=" << (status.frontViewStreamId.empty() ? "none" : status.frontViewStreamId)
                  << " stream_index=" << status.frontViewStreamIndex
                  << " stream_count=" << status.frontViewStreamCount
                  << " frame_age_ms=" << status.frontViewFrameAgeMs
                  << " acquire_ms=" << status.frontViewAcquisitionLatencyMs
                  << " process_ms=" << status.frontViewProcessingLatencyMs
                  << " render_ms=" << status.frontViewRenderLatencyMs
                  << " latency_ms=" << status.frontViewLatencyMs
                  << " dropped=" << status.frontViewDroppedFrames
                  << " drop_reason=" << (status.frontViewDropReason.empty() ? "none" : status.frontViewDropReason)
                  << " stab_mode=" << (status.frontViewStabilizationMode.empty() ? "none" : status.frontViewStabilizationMode)
                  << " stab_active=" << (status.frontViewStabilizationActive ? "y" : "n")
                  << " stab_error_deg=" << status.frontViewStabilizationErrorDeg
                  << " gimbal_yaw_deg=" << status.frontViewGimbalYawDeg
                  << " gimbal_pitch_deg=" << status.frontViewGimbalPitchDeg
                  << " spoof=" << (status.frontViewSpoofActive ? "y" : "n")
                  << " conf=" << status.frontViewConfidence
                  << "\n";
    }
    if (!status.denialReason.empty())
    {
        std::cout << ui::buildDenialBanner(status.denialReason) << "\n";
    }
    std::cout << "Abort: type 'x' then Enter to stop the current run.\n";
}

bool consumeAbortRequest()
{
    if (!inputStreamAvailable())
    {
        return false;
    }
    if (std::cin.rdbuf()->in_avail() <= 0)
    {
        return false;
    }
    std::string input;
    if (!std::getline(std::cin, input))
    {
        return false;
    }
    if (input == "x" || input == "X")
    {
        return true;
    }
    return false;
}


std::string profileName(SimConfig::PlatformProfile profile)
{
    switch (profile)
    {
    case SimConfig::PlatformProfile::Air:
        return "air";
    case SimConfig::PlatformProfile::Ground:
        return "ground";
    case SimConfig::PlatformProfile::Maritime:
        return "maritime";
    case SimConfig::PlatformProfile::Space:
        return "space";
    case SimConfig::PlatformProfile::Handheld:
        return "handheld";
    case SimConfig::PlatformProfile::FixedSite:
        return "fixed_site";
    case SimConfig::PlatformProfile::Subsea:
        return "subsea";
    case SimConfig::PlatformProfile::Base:
    default:
        return "base";
    }
}

bool profileFromName(const std::string &name, SimConfig::PlatformProfile &profile)
{
    std::string lowered = name;
    std::transform(lowered.begin(), lowered.end(), lowered.begin(), [](unsigned char ch)
    {
        return static_cast<char>(std::tolower(ch));
    });
    if (lowered == "base")
    {
        profile = SimConfig::PlatformProfile::Base;
        return true;
    }
    if (lowered == "air")
    {
        profile = SimConfig::PlatformProfile::Air;
        return true;
    }
    if (lowered == "ground")
    {
        profile = SimConfig::PlatformProfile::Ground;
        return true;
    }
    if (lowered == "maritime")
    {
        profile = SimConfig::PlatformProfile::Maritime;
        return true;
    }
    if (lowered == "space")
    {
        profile = SimConfig::PlatformProfile::Space;
        return true;
    }
    if (lowered == "handheld")
    {
        profile = SimConfig::PlatformProfile::Handheld;
        return true;
    }
    if (lowered == "fixed_site")
    {
        profile = SimConfig::PlatformProfile::FixedSite;
        return true;
    }
    if (lowered == "subsea")
    {
        profile = SimConfig::PlatformProfile::Subsea;
        return true;
    }
    return false;
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

std::vector<std::string> supportedPlatformProfiles()
{
    return {"base", "air", "ground", "maritime", "space", "handheld", "fixed_site", "subsea"};
}

bool isOfficialAdapterProfile(const std::string &profile)
{
    return profile == "air" || profile == "ground" || profile == "maritime" ||
           profile == "space" || profile == "handheld" || profile == "fixed_site" ||
           profile == "subsea";
}

std::string resolveProjectPath(const std::string &relativePath)
{
    namespace fs = std::filesystem;
    fs::path cursor = fs::current_path();
    for (int depth = 0; depth < 8; ++depth)
    {
        fs::path candidate = cursor / relativePath;
        std::error_code ec;
        if (fs::exists(candidate, ec))
        {
            return candidate.lexically_normal().string();
        }
        if (!cursor.has_parent_path())
        {
            break;
        }
        cursor = cursor.parent_path();
    }
    return "";
}

std::vector<SensorUiSnapshot> buildProfileSensors(const std::vector<std::string> &permittedSensors)
{
    std::vector<SensorUiSnapshot> sensors;
    sensors.reserve(permittedSensors.size());
    for (const auto &name : permittedSensors)
    {
        SensorUiSnapshot snapshot;
        snapshot.name = name;
        snapshot.available = true;
        snapshot.healthy = true;
        snapshot.hasMeasurement = true;
        snapshot.timeSinceLastValid = 0.0;
        snapshot.confidence = 1.0;
        snapshot.lastError = "";
        sensors.push_back(snapshot);
    }
    return sensors;
}

std::string jsonEscape(const std::string &value)
{
    std::ostringstream out;
    for (char ch : value)
    {
        switch (ch)
        {
        case '\\':
            out << "\\\\";
            break;
        case '"':
            out << "\\\"";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << ch;
            break;
        }
    }
    return out.str();
}

std::string networkAidModeName(SimConfig::NetworkAidMode mode)
{
    switch (mode)
    {
    case SimConfig::NetworkAidMode::Allow:
        return "allow";
    case SimConfig::NetworkAidMode::TestOnly:
        return "test_only";
    case SimConfig::NetworkAidMode::Deny:
    default:
        return "deny";
    }
}

std::string modulesToString(const std::vector<std::string> &modules)
{
    if (modules.empty())
    {
        return "none";
    }
    std::ostringstream out;
    for (size_t idx = 0; idx < modules.size(); ++idx)
    {
        out << modules[idx];
        if (idx + 1 < modules.size())
        {
            out << ",";
        }
    }
    return out.str();
}

std::string buildAuthStatus(const SimConfig &config)
{
    std::ostringstream out;
    out << "net=" << networkAidModeName(config.policy.networkAidMode)
        << " role=" << config.policy.activeRole;
    if (config.policy.overrideRequired)
    {
        out << " override=required";
    }
    else
    {
        out << " override=none";
    }
    if (config.policy.authorization.configured())
    {
        out << " auth_bundle=present";
    }
    else
    {
        out << " auth_bundle=missing";
    }
    return out.str();
}

std::string provenanceModeName(SimConfig::ProvenanceMode mode)
{
    switch (mode)
    {
    case SimConfig::ProvenanceMode::Operational:
        return "operational";
    case SimConfig::ProvenanceMode::Simulation:
        return "simulation";
    case SimConfig::ProvenanceMode::Test:
        return "test";
    default:
        return "unknown";
    }
}

std::string provenanceActionName(SimConfig::UnknownProvenanceAction action)
{
    switch (action)
    {
    case SimConfig::UnknownProvenanceAction::Hold:
        return "hold";
    case SimConfig::UnknownProvenanceAction::Deny:
    default:
        return "deny";
    }
}

std::string buildProvenanceStatus(const SimConfig &config)
{
    std::ostringstream out;
    out << "run=" << provenanceModeName(config.provenance.runMode)
        << " allow_mixed=" << (config.provenance.allowMixed ? "y" : "n")
        << " unknown=" << provenanceActionName(config.provenance.unknownAction)
        << " allowed=";
    if (config.provenance.allowedInputs.empty())
    {
        out << "none";
        return out.str();
    }
    for (size_t idx = 0; idx < config.provenance.allowedInputs.size(); ++idx)
    {
        out << provenanceModeName(config.provenance.allowedInputs[idx]);
        if (idx + 1 < config.provenance.allowedInputs.size())
        {
            out << ",";
        }
    }
    return out.str();
}

void applyActiveRoleUiPreset(SimConfig &config)
{
    auto presetIt = config.policy.roleUiPresets.find(config.policy.activeRole);
    if (presetIt == config.policy.roleUiPresets.end())
    {
        return;
    }
    const auto &preset = presetIt->second;
    if (!preset.uiSurface.empty())
    {
        config.adapter.uiSurface = preset.uiSurface;
    }
    if (preset.hasFrontViewEnabled)
    {
        config.frontView.enabled = preset.frontViewEnabled;
    }
    if (preset.hasFrontViewFamilies)
    {
        config.frontView.displayFamilies = preset.frontViewFamilies;
    }
}

void setFrontViewStatusDefaults(const SimConfig &config)
{
    uiContext.status.frontViewMode = config.frontView.enabled
                                         ? (config.frontView.displayFamilies.empty() ? "none" : config.frontView.displayFamilies.front())
                                         : "none";
    uiContext.status.frontViewViewState = "none";
    uiContext.status.frontViewFrameId.clear();
    uiContext.status.frontViewSourceId.clear();
    uiContext.status.frontViewSensorType.clear();
    uiContext.status.frontViewSequence = 0;
    uiContext.status.frontViewTimestampMs = 0;
    uiContext.status.frontViewFrameAgeMs = 0.0;
    uiContext.status.frontViewAcquisitionLatencyMs = 0.0;
    uiContext.status.frontViewProcessingLatencyMs = 0.0;
    uiContext.status.frontViewRenderLatencyMs = 0.0;
    uiContext.status.frontViewLatencyMs = 0.0;
    uiContext.status.frontViewDroppedFrames = 0;
    uiContext.status.frontViewDropReason.clear();
    uiContext.status.frontViewSpoofActive = false;
    uiContext.status.frontViewConfidence = 0.0;
    uiContext.status.frontViewProvenance = (config.frontView.enabled && config.frontView.spoofEnabled) ? "simulation" : "unknown";
    uiContext.status.frontViewAuthStatus = config.frontView.enabled ? "authorized" : "not_configured";
    uiContext.status.frontViewStreamId = config.frontView.enabled
                                             ? (config.frontView.streamIds.empty() ? "primary" : config.frontView.streamIds.front())
                                             : "primary";
    uiContext.status.frontViewStreamIndex = 0;
    uiContext.status.frontViewStreamCount = 0;
    uiContext.status.frontViewMaxConcurrentViews = static_cast<unsigned int>(config.frontView.maxConcurrentViews);
    uiContext.status.frontViewStabilizationMode = config.frontView.stabilizationMode;
    uiContext.status.frontViewStabilizationActive = config.frontView.stabilizationEnabled;
    uiContext.status.frontViewStabilizationErrorDeg = 0.0;
    uiContext.status.frontViewGimbalYawDeg = 0.0;
    uiContext.status.frontViewGimbalPitchDeg = 0.0;
    uiContext.status.frontViewGimbalYawRateDegPerSec = 0.0;
    uiContext.status.frontViewGimbalPitchRateDegPerSec = 0.0;
    uiContext.status.frontViewStreams.clear();
}

void upsertFrontViewStreamRecord(const FrontViewFrameResult &frame)
{
    ExternalIoFrontViewStreamRecord record;
    record.streamId = frame.streamId;
    record.activeMode = frame.activeMode;
    record.frameId = frame.frameId;
    record.sensorType = frame.sensorType;
    record.sequence = frame.sequence;
    record.timestampMs = frame.timestampMs;
    record.frameAgeMs = frame.frameAgeMs;
    record.latencyMs = frame.latencyMs;
    record.confidence = frame.confidence;
    record.stabilizationMode = frame.stabilizationMode;
    record.stabilizationActive = frame.stabilizationActive;

    for (auto &existing : uiContext.status.frontViewStreams)
    {
        if (existing.streamId == record.streamId)
        {
            existing = record;
            return;
        }
    }
    uiContext.status.frontViewStreams.push_back(record);
}

void applyFrontViewFrameResult(const FrontViewFrameResult &frame)
{
    uiContext.status.frontViewMode = frame.activeMode;
    uiContext.status.frontViewViewState = frame.viewState;
    uiContext.status.frontViewFrameId = frame.frameId;
    uiContext.status.frontViewSourceId = frame.sourceId;
    uiContext.status.frontViewSensorType = frame.sensorType;
    uiContext.status.frontViewSequence = frame.sequence;
    uiContext.status.frontViewTimestampMs = frame.timestampMs;
    uiContext.status.frontViewFrameAgeMs = frame.frameAgeMs;
    uiContext.status.frontViewAcquisitionLatencyMs = frame.acquisitionLatencyMs;
    uiContext.status.frontViewProcessingLatencyMs = frame.processingLatencyMs;
    uiContext.status.frontViewRenderLatencyMs = frame.renderLatencyMs;
    uiContext.status.frontViewLatencyMs = frame.latencyMs;
    uiContext.status.frontViewDroppedFrames += frame.droppedFrames;
    uiContext.status.frontViewDropReason = frame.dropReason;
    uiContext.status.frontViewSpoofActive = frame.spoofActive;
    uiContext.status.frontViewConfidence = frame.confidence;
    uiContext.status.frontViewProvenance = frame.provenance;
    uiContext.status.frontViewAuthStatus = frame.authStatus;
    uiContext.status.frontViewStreamId = frame.streamId;
    uiContext.status.frontViewStreamIndex = frame.streamIndex;
    uiContext.status.frontViewStreamCount = frame.streamCount;
    uiContext.status.frontViewMaxConcurrentViews = frame.maxConcurrentViews;
    uiContext.status.frontViewStabilizationMode = frame.stabilizationMode;
    uiContext.status.frontViewStabilizationActive = frame.stabilizationActive;
    uiContext.status.frontViewStabilizationErrorDeg = frame.stabilizationErrorDeg;
    uiContext.status.frontViewGimbalYawDeg = frame.gimbalYawDeg;
    uiContext.status.frontViewGimbalPitchDeg = frame.gimbalPitchDeg;
    uiContext.status.frontViewGimbalYawRateDegPerSec = frame.gimbalYawRateDegPerSec;
    uiContext.status.frontViewGimbalPitchRateDegPerSec = frame.gimbalPitchRateDegPerSec;
    upsertFrontViewStreamRecord(frame);
}

void updateStatusFromConfig(const SimConfig &config)
{
    uiContext.status.platformProfile = profileName(config.platformProfile);
    uiContext.status.parentProfile = config.hasParentProfile ? profileName(config.parentProfile) : "none";
    uiContext.status.childModules = modulesToString(config.childModules);
    uiContext.status.authStatus = buildAuthStatus(config);
    uiContext.status.provenanceStatus = buildProvenanceStatus(config);
    uiContext.status.adapterId = config.adapter.id;
    uiContext.status.adapterVersion = config.adapter.version;
    uiContext.status.adapterSurface = config.adapter.uiSurface.empty() ? "tui" : config.adapter.uiSurface;
    uiContext.status.adapterStatus = config.adapter.id.empty() ? "none" : "unverified";
    uiContext.status.adapterReason = "";
    uiContext.status.adapterApproval = "";
    uiContext.status.adapterContext = "";
    uiContext.status.adapterFields = "";
    if (uiContext.status.loggingStatus.empty())
    {
        uiContext.status.loggingStatus = "unknown";
    }
    uiContext.status.seed = config.seed;
    uiContext.status.deterministic = true;
    uiContext.status.contributors = "";
    uiContext.status.modeConfidence = 0.0;
    uiContext.status.disqualifiedSources = "";
    uiContext.status.lockoutStatus = "";
    uiContext.status.ladderStatus = "";
    uiContext.status.sensorStatusSummary = "";
    uiContext.status.concurrencyStatus = (config.frontView.enabled && config.frontView.threadingEnabled)
                                             ? ("front_view_threads=" + std::to_string(config.frontView.threadingMaxWorkers))
                                             : "none";
    uiContext.status.decisionReason = "";
    uiContext.status.denialReason = "";
    setFrontViewStatusDefaults(config);
    if (uiContext.status.activeSource.empty())
    {
        uiContext.status.activeSource = "none";
    }
    uiContext.lastSensors.clear();
}

bool applyPlatformProfile(SimConfig::PlatformProfile profile, std::string &reason)
{
    if (!uiContext.configLoaded)
    {
        reason = "config_invalid";
        return false;
    }

    SimConfig updated = uiContext.config;
    updated.platformProfile = profile;
    updated.hasParentProfile = false;
    updated.parentProfile = SimConfig::PlatformProfile::Base;
    updated.childModules.clear();
    updated.permittedSensors = defaultSensorsForProfile(profile);

    const std::string profileLabel = profileName(profile);
    if (isOfficialAdapterProfile(profileLabel))
    {
        updated.adapter.id = profileLabel;
        updated.adapter.version = "1.0.0";
        if (updated.adapter.uiSurface.empty())
        {
            updated.adapter.uiSurface = "tui";
        }
        if (updated.adapter.manifestPath.empty())
        {
            updated.adapter.manifestPath = resolveProjectPath("adapters/official/" + profileLabel + "/manifest.json");
        }
        if (updated.adapter.allowlistPath.empty())
        {
            updated.adapter.allowlistPath = resolveProjectPath("adapters/allowlist.json");
        }
    }
    else
    {
        updated.adapter.id.clear();
        updated.adapter.version.clear();
        updated.adapter.manifestPath.clear();
        updated.adapter.allowlistPath.clear();
    }

    applyActiveRoleUiPreset(updated);
    uiContext.config = updated;
    uiContext.seed = updated.seed;
    uiContext.rng.seed(uiContext.seed);
    updateStatusFromConfig(updated);

    tools::AdapterUiSnapshot snapshot = tools::loadAdapterUiSnapshot(updated);
    uiContext.status.adapterId = snapshot.adapterId;
    uiContext.status.adapterVersion = snapshot.adapterVersion;
    uiContext.status.adapterSurface = snapshot.surface.empty() ? "tui" : snapshot.surface;
    uiContext.status.adapterStatus = snapshot.status;
    uiContext.status.adapterReason = snapshot.reason;
    uiContext.status.adapterApproval = snapshot.approvedBy.empty() ? "" : (snapshot.approvedBy + "@" + snapshot.approvalDate + " sig=" + snapshot.signatureAlgorithm);
    uiContext.status.adapterContext = snapshot.contextVersionSummary;
    uiContext.status.adapterFields = ui::formatAdapterFieldSummary(snapshot.fields);

    if (snapshot.status != "ok" && snapshot.status != "none")
    {
        uiContext.status.denialReason = snapshot.reason;
        reason = snapshot.reason;
        return false;
    }

    uiContext.status.denialReason.clear();
    reason = "ok";
    return true;
}
} // namespace

bool uiEnsureAuditHealthy(const std::string &context)
{
    return ensureAuditHealthy(context);
}

bool uiHasPermission(const std::string &permission)
{
    return hasPermission(permission);
}

void uiRenderStatusBanner(const std::string &context)
{
    renderStatusBanner(context);
}

bool initializeUiContext(const std::string &configPath)
{
    ConfigResult loaded = loadSimConfig(configPath);
    if (!loaded.ok)
    {
        uiContext.configLoaded = false;
        uiContext.lastSensors.clear();
        uiContext.status.platformProfile = "base";
        uiContext.status.parentProfile = "none";
        uiContext.status.childModules = "none";
        uiContext.status.authStatus = "config_invalid";
        uiContext.status.denialReason = "config_invalid";
        uiContext.status.adapterId = "";
        uiContext.status.adapterVersion = "";
        uiContext.status.adapterSurface = "tui";
        uiContext.status.adapterStatus = "none";
        uiContext.status.adapterReason = "";
        uiContext.status.adapterApproval = "";
        uiContext.status.adapterContext = "";
        uiContext.status.adapterFields = "";
        uiContext.status.frontViewMode = "none";
        uiContext.status.frontViewViewState = "none";
        uiContext.status.frontViewFrameId = "";
        uiContext.status.frontViewSensorType = "";
        uiContext.status.frontViewSequence = 0;
        uiContext.status.frontViewLatencyMs = 0.0;
        uiContext.status.frontViewDroppedFrames = 0;
        uiContext.status.frontViewDropReason = "";
        uiContext.status.frontViewSpoofActive = false;
        uiContext.status.frontViewConfidence = 0.0;
        uiContext.status.frontViewProvenance = "unknown";
        uiContext.status.frontViewAuthStatus = "not_configured";
        uiContext.status.seed = uiContext.seed;
        uiContext.status.deterministic = true;
        if (uiContext.status.activeSource.empty())
        {
            uiContext.status.activeSource = "none";
        }
        return false;
    }

    uiContext.config = loaded.config;
    applyActiveRoleUiPreset(uiContext.config);
    uiContext.configLoaded = true;
    uiContext.seed = uiContext.config.seed;
    uiContext.rng.seed(uiContext.seed);
    updateStatusFromConfig(uiContext.config);
    {
        tools::AdapterUiSnapshot snapshot = tools::loadAdapterUiSnapshot(uiContext.config);
        uiContext.status.adapterId = snapshot.adapterId;
        uiContext.status.adapterVersion = snapshot.adapterVersion;
        uiContext.status.adapterSurface = snapshot.surface.empty() ? "tui" : snapshot.surface;
        uiContext.status.adapterStatus = snapshot.status;
        uiContext.status.adapterReason = snapshot.reason;
        uiContext.status.adapterApproval = snapshot.approvedBy.empty() ? "" : (snapshot.approvedBy + "@" + snapshot.approvalDate + " sig=" + snapshot.signatureAlgorithm);
        uiContext.status.adapterContext = snapshot.contextVersionSummary;
        uiContext.status.adapterFields = ui::formatAdapterFieldSummary(snapshot.fields);
        if (snapshot.status != "ok" && snapshot.status != "none")
        {
            uiContext.status.denialReason = snapshot.reason;
        }
    }
    tools::setAuditRunContext("", uiContext.config.version, uiContext.config.seed);
    return true;
}

const UiStatus &getUiStatus()
{
    return uiContext.status;
}

std::vector<std::string> uiListPlatformProfiles()
{
    return supportedPlatformProfiles();
}

PlatformSuiteResult uiRunPlatformSuite(const std::string &profileNameValue)
{
    PlatformSuiteResult result;
    result.profile = profileNameValue;

    SimConfig::PlatformProfile profile = SimConfig::PlatformProfile::Base;
    if (!profileFromName(profileNameValue, profile))
    {
        result.reason = "platform_profile_invalid";
        setUiDenialReason(result.reason);
        return result;
    }

    result.profile = profileName(profile);
    std::string applyReason;
    if (!applyPlatformProfile(profile, applyReason))
    {
        result.reason = applyReason.empty() ? "platform_profile_invalid" : applyReason;
        setUiDenialReason(result.reason);
        return result;
    }

    result.sensorsValidated = !uiContext.config.permittedSensors.empty();

    ModeDecisionDetail detail;
    detail.selectedMode = result.sensorsValidated ? uiContext.config.permittedSensors.front() : "hold";
    detail.contributors = {detail.selectedMode};
    detail.confidence = 1.0;
    detail.reason = "platform_suite";
    detail.downgradeReason = "";

    std::vector<SensorUiSnapshot> sensors = buildProfileSensors(uiContext.config.permittedSensors);
    setUiDenialReason("");
    updateUiFromModeDecision(detail, sensors);

    result.modeOutputValidated = !uiContext.status.ladderStatus.empty() && !uiContext.status.sensorStatusSummary.empty();
    result.adapterValidated = (uiContext.status.adapterStatus == "ok" || uiContext.status.adapterStatus == "none");
    result.pass = result.sensorsValidated && result.adapterValidated && result.modeOutputValidated;
    if (result.pass)
    {
        result.reason = "ok";
        setUiDenialReason("");
    }
    else if (!result.adapterValidated)
    {
        result.reason = uiContext.status.adapterReason.empty() ? "adapter_invalid" : uiContext.status.adapterReason;
        setUiDenialReason(result.reason);
    }
    else if (!result.sensorsValidated)
    {
        result.reason = "platform_sensors_invalid";
        setUiDenialReason(result.reason);
    }
    else
    {
        result.reason = "platform_mode_output_invalid";
        setUiDenialReason(result.reason);
    }

    return result;
}

std::vector<PlatformSuiteResult> uiRunAllPlatformSuites()
{
    std::vector<PlatformSuiteResult> results;
    for (const auto &profile : supportedPlatformProfiles())
    {
        results.push_back(uiRunPlatformSuite(profile));
    }
    return results;
}

std::vector<std::string> uiListFrontViewDisplayModes()
{
    return frontViewSupportedModes();
}

bool uiRunFrontViewDisplaySuite(bool cycleAllModes, std::string &reason)
{
    if (!uiContext.configLoaded)
    {
        reason = "config_invalid";
        setUiDenialReason(reason);
        return false;
    }

    std::mt19937 frontViewRng(uiContext.config.frontView.spoofSeed);
    std::vector<FrontViewFrameResult> frames;
    if (!frontViewCycleFrames(uiContext.config.frontView, cycleAllModes, frontViewRng, frames, reason))
    {
        setUiDenialReason(reason);
        return false;
    }
    if (frames.empty())
    {
        reason = "front_view_cycle_empty";
        setUiDenialReason(reason);
        return false;
    }

    uiContext.status.frontViewDroppedFrames = 0;
    uiContext.status.frontViewDropReason.clear();
    uiContext.status.frontViewStreams.clear();
    for (const auto &frame : frames)
    {
        applyFrontViewFrameResult(frame);
        setUiActiveSource(frame.activeMode);
        setUiContributors({frame.sensorType + ":" + frame.streamId});
        setUiModeConfidence(frame.confidence);
        setUiDecisionReason("front_view_cycle");
    }

    if (uiContext.status.frontViewDropReason.empty())
    {
        setUiDenialReason("");
        reason = "ok";
    }
    else
    {
        setUiDenialReason(uiContext.status.frontViewDropReason);
        reason = uiContext.status.frontViewDropReason;
    }
    return true;
}

ExternalIoEnvelope uiBuildExternalIoEnvelope()
{
    ExternalIoEnvelope envelope;
    envelope.metadata.schemaVersion = "1.0.0";
    envelope.metadata.interfaceId = "airtrace.external_io";
    envelope.metadata.platformProfile = uiContext.status.platformProfile;
    envelope.metadata.adapterId = uiContext.status.adapterId;
    envelope.metadata.adapterVersion = uiContext.status.adapterVersion;
    envelope.metadata.uiSurface = uiContext.status.adapterSurface.empty() ? "tui" : uiContext.status.adapterSurface;
    envelope.metadata.seed = uiContext.status.seed;
    envelope.metadata.deterministic = uiContext.status.deterministic;

    std::vector<SensorUiSnapshot> sensors = uiContext.lastSensors;
    if (sensors.empty())
    {
        sensors = buildProfileSensors(uiContext.config.permittedSensors);
    }
    for (const auto &sensor : sensors)
    {
        ExternalIoSensorRecord record;
        record.sensorId = sensor.name;
        record.available = sensor.available;
        record.healthy = sensor.healthy;
        record.hasMeasurement = sensor.hasMeasurement;
        record.freshnessSeconds = sensor.timeSinceLastValid;
        record.confidence = sensor.confidence;
        record.lastError = sensor.lastError;
        envelope.sensors.push_back(record);
    }

    envelope.mode.activeMode = uiContext.status.activeSource;
    if (!uiContext.status.contributors.empty())
    {
        std::stringstream stream(uiContext.status.contributors);
        std::string contributor;
        while (std::getline(stream, contributor, ','))
        {
            if (!contributor.empty())
            {
                envelope.mode.contributors.push_back(contributor);
            }
        }
    }
    envelope.mode.confidence = uiContext.status.modeConfidence;
    envelope.mode.decisionReason = uiContext.status.decisionReason;
    envelope.mode.denialReason = uiContext.status.denialReason;
    envelope.mode.ladderStatus = uiContext.status.ladderStatus;
    envelope.disqualifiedSources = uiContext.status.disqualifiedSources;
    envelope.lockoutStatus = uiContext.status.lockoutStatus;
    envelope.authStatus = uiContext.status.authStatus;
    envelope.provenanceStatus = uiContext.status.provenanceStatus;
    envelope.loggingStatus = uiContext.status.loggingStatus;
    envelope.adapterStatus = uiContext.status.adapterStatus;
    envelope.adapterReason = uiContext.status.adapterReason;
    envelope.adapterFields = uiContext.status.adapterFields;
    envelope.frontView.activeMode = uiContext.status.frontViewMode;
    envelope.frontView.viewState = uiContext.status.frontViewViewState;
    envelope.frontView.frameId = uiContext.status.frontViewFrameId;
    envelope.frontView.sourceId = uiContext.status.frontViewSourceId;
    envelope.frontView.sensorType = uiContext.status.frontViewSensorType;
    envelope.frontView.sequence = uiContext.status.frontViewSequence;
    envelope.frontView.timestampMs = uiContext.status.frontViewTimestampMs;
    envelope.frontView.frameAgeMs = uiContext.status.frontViewFrameAgeMs;
    envelope.frontView.acquisitionLatencyMs = uiContext.status.frontViewAcquisitionLatencyMs;
    envelope.frontView.processingLatencyMs = uiContext.status.frontViewProcessingLatencyMs;
    envelope.frontView.renderLatencyMs = uiContext.status.frontViewRenderLatencyMs;
    envelope.frontView.latencyMs = uiContext.status.frontViewLatencyMs;
    envelope.frontView.droppedFrames = uiContext.status.frontViewDroppedFrames;
    envelope.frontView.dropReason = uiContext.status.frontViewDropReason;
    envelope.frontView.spoofActive = uiContext.status.frontViewSpoofActive;
    envelope.frontView.confidence = uiContext.status.frontViewConfidence;
    envelope.frontView.provenance = uiContext.status.frontViewProvenance;
    envelope.frontView.authStatus = uiContext.status.frontViewAuthStatus;
    envelope.frontView.streamId = uiContext.status.frontViewStreamId;
    envelope.frontView.streamIndex = uiContext.status.frontViewStreamIndex;
    envelope.frontView.streamCount = uiContext.status.frontViewStreamCount;
    envelope.frontView.maxConcurrentViews = uiContext.status.frontViewMaxConcurrentViews;
    envelope.frontView.stabilizationMode = uiContext.status.frontViewStabilizationMode;
    envelope.frontView.stabilizationActive = uiContext.status.frontViewStabilizationActive;
    envelope.frontView.stabilizationErrorDeg = uiContext.status.frontViewStabilizationErrorDeg;
    envelope.frontView.gimbalYawDeg = uiContext.status.frontViewGimbalYawDeg;
    envelope.frontView.gimbalPitchDeg = uiContext.status.frontViewGimbalPitchDeg;
    envelope.frontView.gimbalYawRateDegPerSec = uiContext.status.frontViewGimbalYawRateDegPerSec;
    envelope.frontView.gimbalPitchRateDegPerSec = uiContext.status.frontViewGimbalPitchRateDegPerSec;
    envelope.frontViewStreams = uiContext.status.frontViewStreams;
    return envelope;
}

std::string uiBuildExternalIoEnvelopeJson()
{
    const ExternalIoEnvelope envelope = uiBuildExternalIoEnvelope();
    std::ostringstream out;
    out << "{";
    out << "\"schema_version\":\"" << jsonEscape(envelope.metadata.schemaVersion) << "\",";
    out << "\"interface_id\":\"" << jsonEscape(envelope.metadata.interfaceId) << "\",";
    out << "\"metadata\":{";
    out << "\"platform_profile\":\"" << jsonEscape(envelope.metadata.platformProfile) << "\",";
    out << "\"adapter_id\":\"" << jsonEscape(envelope.metadata.adapterId) << "\",";
    out << "\"adapter_version\":\"" << jsonEscape(envelope.metadata.adapterVersion) << "\",";
    out << "\"ui_surface\":\"" << jsonEscape(envelope.metadata.uiSurface) << "\",";
    out << "\"seed\":" << envelope.metadata.seed << ",";
    out << "\"deterministic\":" << (envelope.metadata.deterministic ? "true" : "false");
    out << "},";
    out << "\"mode\":{";
    out << "\"active\":\"" << jsonEscape(envelope.mode.activeMode) << "\",";
    out << "\"confidence\":" << std::fixed << std::setprecision(3) << envelope.mode.confidence << ",";
    out << "\"decision_reason\":\"" << jsonEscape(envelope.mode.decisionReason) << "\",";
    out << "\"denial_reason\":\"" << jsonEscape(envelope.mode.denialReason) << "\",";
    out << "\"ladder_status\":\"" << jsonEscape(envelope.mode.ladderStatus) << "\",";
    out << "\"contributors\":[";
    for (size_t idx = 0; idx < envelope.mode.contributors.size(); ++idx)
    {
        out << "\"" << jsonEscape(envelope.mode.contributors[idx]) << "\"";
        if (idx + 1 < envelope.mode.contributors.size())
        {
            out << ",";
        }
    }
    out << "]";
    out << "},";
    out << "\"sensors\":[";
    for (size_t idx = 0; idx < envelope.sensors.size(); ++idx)
    {
        const auto &sensor = envelope.sensors[idx];
        out << "{";
        out << "\"id\":\"" << jsonEscape(sensor.sensorId) << "\",";
        out << "\"available\":" << (sensor.available ? "true" : "false") << ",";
        out << "\"healthy\":" << (sensor.healthy ? "true" : "false") << ",";
        out << "\"has_measurement\":" << (sensor.hasMeasurement ? "true" : "false") << ",";
        out << "\"freshness_seconds\":" << std::fixed << std::setprecision(3) << sensor.freshnessSeconds << ",";
        out << "\"confidence\":" << std::fixed << std::setprecision(3) << sensor.confidence << ",";
        out << "\"last_error\":\"" << jsonEscape(sensor.lastError) << "\"";
        out << "}";
        if (idx + 1 < envelope.sensors.size())
        {
            out << ",";
        }
    }
    out << "],";
    out << "\"front_view\":{";
    out << "\"active_mode\":\"" << jsonEscape(envelope.frontView.activeMode) << "\",";
    out << "\"view_state\":\"" << jsonEscape(envelope.frontView.viewState) << "\",";
    out << "\"frame_id\":\"" << jsonEscape(envelope.frontView.frameId) << "\",";
    out << "\"source_id\":\"" << jsonEscape(envelope.frontView.sourceId) << "\",";
    out << "\"sensor_type\":\"" << jsonEscape(envelope.frontView.sensorType) << "\",";
    out << "\"sequence\":" << envelope.frontView.sequence << ",";
    out << "\"timestamp_ms\":" << envelope.frontView.timestampMs << ",";
    out << "\"frame_age_ms\":" << std::fixed << std::setprecision(3) << envelope.frontView.frameAgeMs << ",";
    out << "\"acquisition_latency_ms\":" << std::fixed << std::setprecision(3) << envelope.frontView.acquisitionLatencyMs << ",";
    out << "\"processing_latency_ms\":" << std::fixed << std::setprecision(3) << envelope.frontView.processingLatencyMs << ",";
    out << "\"render_latency_ms\":" << std::fixed << std::setprecision(3) << envelope.frontView.renderLatencyMs << ",";
    out << "\"latency_ms\":" << std::fixed << std::setprecision(3) << envelope.frontView.latencyMs << ",";
    out << "\"dropped_frames\":" << envelope.frontView.droppedFrames << ",";
    out << "\"drop_reason\":\"" << jsonEscape(envelope.frontView.dropReason) << "\",";
    out << "\"spoof_active\":" << (envelope.frontView.spoofActive ? "true" : "false") << ",";
    out << "\"confidence\":" << std::fixed << std::setprecision(3) << envelope.frontView.confidence << ",";
    out << "\"provenance\":\"" << jsonEscape(envelope.frontView.provenance) << "\",";
    out << "\"auth_status\":\"" << jsonEscape(envelope.frontView.authStatus) << "\",";
    out << "\"stream_id\":\"" << jsonEscape(envelope.frontView.streamId) << "\",";
    out << "\"stream_index\":" << envelope.frontView.streamIndex << ",";
    out << "\"stream_count\":" << envelope.frontView.streamCount << ",";
    out << "\"max_concurrent_views\":" << envelope.frontView.maxConcurrentViews << ",";
    out << "\"stabilization_mode\":\"" << jsonEscape(envelope.frontView.stabilizationMode) << "\",";
    out << "\"stabilization_active\":" << (envelope.frontView.stabilizationActive ? "true" : "false") << ",";
    out << "\"stabilization_error_deg\":" << std::fixed << std::setprecision(3) << envelope.frontView.stabilizationErrorDeg << ",";
    out << "\"gimbal_yaw_deg\":" << std::fixed << std::setprecision(3) << envelope.frontView.gimbalYawDeg << ",";
    out << "\"gimbal_pitch_deg\":" << std::fixed << std::setprecision(3) << envelope.frontView.gimbalPitchDeg << ",";
    out << "\"gimbal_yaw_rate_deg_s\":" << std::fixed << std::setprecision(3) << envelope.frontView.gimbalYawRateDegPerSec << ",";
    out << "\"gimbal_pitch_rate_deg_s\":" << std::fixed << std::setprecision(3) << envelope.frontView.gimbalPitchRateDegPerSec;
    out << "},";
    out << "\"front_view_streams\":[";
    for (size_t idx = 0; idx < envelope.frontViewStreams.size(); ++idx)
    {
        const auto &stream = envelope.frontViewStreams[idx];
        out << "{";
        out << "\"stream_id\":\"" << jsonEscape(stream.streamId) << "\",";
        out << "\"active_mode\":\"" << jsonEscape(stream.activeMode) << "\",";
        out << "\"frame_id\":\"" << jsonEscape(stream.frameId) << "\",";
        out << "\"sensor_type\":\"" << jsonEscape(stream.sensorType) << "\",";
        out << "\"sequence\":" << stream.sequence << ",";
        out << "\"timestamp_ms\":" << stream.timestampMs << ",";
        out << "\"frame_age_ms\":" << std::fixed << std::setprecision(3) << stream.frameAgeMs << ",";
        out << "\"latency_ms\":" << std::fixed << std::setprecision(3) << stream.latencyMs << ",";
        out << "\"confidence\":" << std::fixed << std::setprecision(3) << stream.confidence << ",";
        out << "\"stabilization_mode\":\"" << jsonEscape(stream.stabilizationMode) << "\",";
        out << "\"stabilization_active\":" << (stream.stabilizationActive ? "true" : "false");
        out << "}";
        if (idx + 1 < envelope.frontViewStreams.size())
        {
            out << ",";
        }
    }
    out << "],";
    out << "\"status\":{";
    out << "\"disqualified_sources\":\"" << jsonEscape(envelope.disqualifiedSources) << "\",";
    out << "\"lockout_status\":\"" << jsonEscape(envelope.lockoutStatus) << "\",";
    out << "\"auth_status\":\"" << jsonEscape(envelope.authStatus) << "\",";
    out << "\"provenance_status\":\"" << jsonEscape(envelope.provenanceStatus) << "\",";
    out << "\"logging_status\":\"" << jsonEscape(envelope.loggingStatus) << "\",";
    out << "\"adapter_status\":\"" << jsonEscape(envelope.adapterStatus) << "\",";
    out << "\"adapter_reason\":\"" << jsonEscape(envelope.adapterReason) << "\",";
    out << "\"adapter_fields\":\"" << jsonEscape(envelope.adapterFields) << "\"";
    out << "}";
    out << "}";
    return out.str();
}

void setUiActiveSource(const std::string &source)
{
    uiContext.status.activeSource = source;
    if (uiContext.status.contributors.empty())
    {
        uiContext.status.contributors = source;
    }
    if (uiContext.status.modeConfidence <= 0.0)
    {
        uiContext.status.modeConfidence = 1.0;
    }
}

void setUiContributors(const std::vector<std::string> &contributors)
{
    std::ostringstream out;
    for (size_t idx = 0; idx < contributors.size(); ++idx)
    {
        out << contributors[idx];
        if (idx + 1 < contributors.size())
        {
            out << ",";
        }
    }
    uiContext.status.contributors = out.str();
}

void setUiModeConfidence(double confidence)
{
    uiContext.status.modeConfidence = confidence;
}

void setUiConcurrencyStatus(const std::string &status)
{
    uiContext.status.concurrencyStatus = status;
}

void setUiDecisionReason(const std::string &reason)
{
    uiContext.status.decisionReason = reason;
}

void setUiDenialReason(const std::string &reason)
{
    uiContext.status.denialReason = reason;
}

void setUiDisqualifiedSources(const std::string &summary)
{
    uiContext.status.disqualifiedSources = summary;
}

void setUiLockoutStatus(const std::string &summary)
{
    uiContext.status.lockoutStatus = summary;
}

void setUiLadderStatus(const std::string &summary)
{
    uiContext.status.ladderStatus = summary;
}

void setUiSensorStatusSummary(const std::string &summary)
{
    uiContext.status.sensorStatusSummary = summary;
}

namespace
{
std::string formatDisqualifiedSources(const std::vector<ModeDecisionDetail::DisqualifiedSource> &sources)
{
    if (sources.empty())
    {
        return "";
    }
    std::ostringstream out;
    for (size_t idx = 0; idx < sources.size(); ++idx)
    {
        const auto &entry = sources[idx];
        out << entry.mode << ":" << entry.source << "=" << entry.reason;
        if (idx + 1 < sources.size())
        {
            out << ";";
        }
    }
    return out.str();
}

std::string formatLockouts(const std::vector<ModeDecisionDetail::LockoutState> &lockouts)
{
    if (lockouts.empty())
    {
        return "";
    }
    std::ostringstream out;
    for (size_t idx = 0; idx < lockouts.size(); ++idx)
    {
        const auto &entry = lockouts[idx];
        out << entry.source << "(steps=" << entry.remainingSteps << ",reason=" << entry.reason << ")";
        if (idx + 1 < lockouts.size())
        {
            out << ";";
        }
    }
    return out.str();
}

std::string formatSensorSummary(std::vector<SensorUiSnapshot> sensors,
                                const std::vector<ModeDecisionDetail::LockoutState> &lockouts)
{
    if (sensors.empty())
    {
        return "";
    }
    std::unordered_map<std::string, ModeDecisionDetail::LockoutState> lockoutBySource;
    for (const auto &entry : lockouts)
    {
        lockoutBySource[entry.source] = entry;
    }
    std::sort(sensors.begin(), sensors.end(), [](const SensorUiSnapshot &a, const SensorUiSnapshot &b)
    {
        return a.name < b.name;
    });
    std::ostringstream out;
    for (size_t idx = 0; idx < sensors.size(); ++idx)
    {
        const auto &sensor = sensors[idx];
        out << sensor.name << "[avail=" << (sensor.available ? "y" : "n")
            << ",health=" << (sensor.healthy ? "y" : "n")
            << ",meas=" << (sensor.hasMeasurement ? "y" : "n")
            << ",age_s=" << std::fixed << std::setprecision(2) << sensor.timeSinceLastValid
            << ",conf=" << std::fixed << std::setprecision(2) << sensor.confidence;
        auto lockoutIt = lockoutBySource.find(sensor.name);
        if (lockoutIt != lockoutBySource.end())
        {
            out << ",lockout=steps:" << lockoutIt->second.remainingSteps
                << ",reason:" << lockoutIt->second.reason;
        }
        if (!sensor.lastError.empty())
        {
            out << ",err=" << sensor.lastError;
        }
        out << "]";
        if (idx + 1 < sensors.size())
        {
            out << ";";
        }
    }
    return out.str();
}

std::string formatLadderStatus(const std::vector<std::string> &ladder, const ModeDecisionDetail &detail)
{
    if (ladder.empty())
    {
        return "";
    }
    std::unordered_map<std::string, std::vector<std::string>> disqualByMode;
    for (const auto &entry : detail.disqualifiedSources)
    {
        disqualByMode[entry.mode].push_back(entry.source + "=" + entry.reason);
    }
    std::ostringstream out;
    bool selectedSeen = false;
    for (size_t idx = 0; idx < ladder.size(); ++idx)
    {
        const std::string &mode = ladder[idx];
        out << mode << ":";
        if (!detail.selectedMode.empty() && mode == detail.selectedMode)
        {
            out << "selected";
            selectedSeen = true;
        }
        else
        {
            auto disqIt = disqualByMode.find(mode);
            if (disqIt != disqualByMode.end())
            {
                out << "disq(";
                const auto &sources = disqIt->second;
                for (size_t sidx = 0; sidx < sources.size(); ++sidx)
                {
                    out << sources[sidx];
                    if (sidx + 1 < sources.size())
                    {
                        out << ",";
                    }
                }
                out << ")";
            }
            else if (selectedSeen)
            {
                out << "skipped";
            }
            else if (!detail.selectedMode.empty())
            {
                out << "unchecked";
            }
            else
            {
                out << "no_selection";
            }
        }
        if (idx + 1 < ladder.size())
        {
            out << ";";
        }
    }
    return out.str();
}

std::vector<std::string> defaultLadderOrder()
{
    return {
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
} // namespace

void updateUiFromModeDecision(const ModeDecisionDetail &detail, const std::vector<SensorUiSnapshot> &sensors)
{
    setUiActiveSource(detail.selectedMode.empty() ? "none" : detail.selectedMode);
    setUiContributors(detail.contributors);
    setUiModeConfidence(detail.confidence);
    setUiDecisionReason(detail.reason);
    if (!detail.downgradeReason.empty())
    {
        setUiDenialReason(detail.downgradeReason);
    }
    setUiDisqualifiedSources(formatDisqualifiedSources(detail.disqualifiedSources));
    setUiLockoutStatus(formatLockouts(detail.lockouts));
    const auto &configuredLadder = uiContext.config.mode.ladderOrder;
    if (configuredLadder.empty())
    {
        setUiLadderStatus(formatLadderStatus(defaultLadderOrder(), detail));
    }
    else
    {
        setUiLadderStatus(formatLadderStatus(configuredLadder, detail));
    }
    setUiSensorStatusSummary(formatSensorSummary(sensors, detail.lockouts));
    uiContext.lastSensors = sensors;
}

void setUiLoggingStatus(const std::string &status)
{
    uiContext.status.loggingStatus = status;
}

void resetUiRng()
{
    uiContext.rng.seed(uiContext.seed);
}

int uiRandomInt(int minValue, int maxValue)
{
    std::uniform_int_distribution<int> dist(minValue, maxValue);
    return dist(uiContext.rng);
}

/****************************************
 *
 *
 * PROGRAM/INTERACTIVITY FUNCTIONS
 *
 *
 *****************************************/

// Monitor function to check for user input without blocking
void monitorExitKey()
{
    while (!exitRequested.load())
    {
        if (!inputStreamAvailable())
        {
            exitRequested.store(true);
            return;
        }
        if (std::cin.rdbuf()->in_avail() <= 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        std::string input;
        if (!std::getline(std::cin, input))
        {
            exitRequested.store(true);
            return;
        }
        if (input == "x" || input == "X")
        {
            exitRequestedByUser.store(true);
            exitRequested.store(true);
            std::cout << "\nExiting to test menu...\n";
        }
    }
}

/****************************************
 *
 *
 * SIMULATION FUNCTIONS
 *
 *
 *****************************************/

void simulateManualConfig(const SimulationData &simData)
{
    resetUiRng();
    setUiActiveSource(simData.mode);
    setUiDecisionReason("manual_mode");
    renderStatusBanner("manual_sim");
    // Use the simData to run the simulation
    Object target(1, "Target", simData.targetPos);
    Object follower(2, "Follower", simData.followerPos);

    Tracker tracker(follower);
    tracker.setTrackingMode(simData.mode);
    tracker.setTarget(target);

    int stepCount = 0;
    while (tracker.isTrackingActive() && (simData.iterations == 0 || stepCount < simData.iterations))
    {
        if (consumeAbortRequest())
        {
            setUiDenialReason("operator_abort");
            tools::logAuditEvent("operator_abort", "manual simulation aborted", simData.mode);
            std::cout << "Operator abort received. Returning to menu.\n";
            break;
        }
        tracker.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / simData.speed)); // Adjust based on speed
        stepCount++;
    }

    std::cout << "\n\nTest simulation finished.\n\n";
}

void simulateDeadReckoning(int speed, int iterations)
{
    resetUiRng();
    setUiActiveSource("dead_reckoning");
    setUiDecisionReason("dead_reckoning_active");
    renderStatusBanner("dead_reckoning");
    Object target(1, "Target", {uiRandomInt(0, 99), uiRandomInt(0, 99)});
    Object follower(2, "Follower", {uiRandomInt(0, 99), uiRandomInt(0, 99)});

    Tracker tracker(follower);
    tracker.setTrackingMode("dead_reckoning");
    tracker.setTarget(target);

    int stepCount = 0;
    std::string simulationLog;
    std::string condensedLog;

    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        if (consumeAbortRequest())
        {
            setUiDenialReason("operator_abort");
            tools::logAuditEvent("operator_abort", "dead_reckoning aborted", "");
            std::cout << "Operator abort received. Returning to menu.\n";
            break;
        }
        tracker.update();

        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        std::cout << "[Iteration " << stepCount << "] Dead Reckoning Mode\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "Target Position (m): (" << targetPos.first << ", " << targetPos.second << ")\n";
        std::cout << "Follower Position (m): (" << followerPos.first << ", " << followerPos.second << ")\n";
        std::cout << "Distance to Target (m): " << distance << "\n";
        std::cout << "--------------------------------------------\n";

        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) + "\n";
        condensedLog += "Iteration " + std::to_string(stepCount) + " - Distance: " + std::to_string(distance) + " m\n";

        if (distance < 0.1)
        {
            std::cout << "\nFollower has reached the target.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\nDead Reckoning simulation finished.\n";
    logSimulationResult("Dead Reckoning", simulationLog, condensedLog);
}

void simulateHeatSeeking(int speed, int iterations)
{
    resetUiRng();
    setUiActiveSource("heat_signature");
    setUiDecisionReason("heat_signature_active");
    renderStatusBanner("heat_seeking");
    // Initialize random positions for target and follower
    Object target(1, "Target", {uiRandomInt(0, 99), uiRandomInt(0, 99)});
    Object follower(2, "Follower", {uiRandomInt(0, 99), uiRandomInt(0, 99)});

    Tracker tracker(follower);
    tracker.setTrackingMode("heat_signature");
    tracker.setTarget(target);

    int stepCount = 0;
    std::string simulationLog;
    std::string condensedLog;

    // Dynamically move target and update heat signature
    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        if (consumeAbortRequest())
        {
            setUiDenialReason("operator_abort");
            tools::logAuditEvent("operator_abort", "heat_seeking aborted", "");
            std::cout << "Operator abort received. Returning to menu.\n";
            break;
        }
        // Random movement for the target simulating real-world data
        int randomX = uiRandomInt(-1, 1); // -1, 0, or 1
        int randomY = uiRandomInt(-1, 1);
        target.moveTo({target.getPosition().first + randomX, target.getPosition().second + randomY});

        // Calculate distance between target and follower
        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        // Simulate a heat signature for test, stronger the closer the follower is
        float heatSignature = 100.0f / (1.0f + static_cast<float>(distance));

        // Update tracker with the heat signature data
        tracker.updateHeatSignature(heatSignature);
        tracker.update();

        // Detailed output for the user
        std::cout << "\n[Iteration " << stepCount << "]\n";
        std::cout << "--------------------------------------------\n";
        std::cout << std::fixed << std::setprecision(2); // Two decimal places for numbers
        std::cout << "Target Position (m): (" << targetPos.first << ", " << targetPos.second << ")\n";
        std::cout << "Follower Position (m): (" << followerPos.first << ", " << followerPos.second << ")\n";
        std::cout << "Distance to Target (m): " << distance << "\n";
        std::cout << "Heat Signature (arb units): " << heatSignature << "\n";
        std::cout << "--------------------------------------------\n";

        // Logging compact information for the text file
        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) +
                         ", Heat Signature: " + std::to_string(heatSignature) + "\n";

        // If distance is extremely small, stop the simulation (the follower "reaches" the target)
        if (distance < 0.1)
        {
            std::cout << "\nFollower has hit the target and stopped.\n";
            break;
        }

        // Sleep based on speed
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\nHeat-seeking simulation finished.\n\n--------------------------------------------\n\n";
    logSimulationResult("Heat Seeking", simulationLog, condensedLog); // Log simulation details
}

void simulateGPSSeeking(int speed, int iterations)
{
    resetUiRng();
    setUiActiveSource("gps");
    setUiDecisionReason("gps_active");
    renderStatusBanner("gps_seek");
    Object target(1, "Target", {uiRandomInt(0, 99), uiRandomInt(0, 99)});
    Object follower(2, "Follower", {uiRandomInt(0, 99), uiRandomInt(0, 99)});

    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    tracker.setTarget(target);

    int stepCount = 0;
    std::string simulationLog;
    std::string condensedLog;

    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        if (consumeAbortRequest())
        {
            setUiDenialReason("operator_abort");
            tools::logAuditEvent("operator_abort", "gps_seek aborted", "");
            std::cout << "Operator abort received. Returning to menu.\n";
            break;
        }
        int randomX = uiRandomInt(-1, 1);
        int randomY = uiRandomInt(-1, 1);
        target.moveTo({target.getPosition().first + randomX, target.getPosition().second + randomY});

        tracker.update();

        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        std::cout << "[Iteration " << stepCount << "] GPS Mode\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "Target GPS Position (m): (" << targetPos.first << ", " << targetPos.second << ")\n";
        std::cout << "Follower GPS Position (m): (" << followerPos.first << ", " << followerPos.second << ")\n";
        std::cout << "Distance to Target (m): " << distance << "\n";
        std::cout << "--------------------------------------------\n";

        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) + "\n";
        condensedLog += "Iteration " + std::to_string(stepCount) + " - Distance: " + std::to_string(distance) + " m\n";

        if (distance < 0.1)
        {
            std::cout << "\nFollower has reached the target.\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "GPS-based simulation finished.\n";
    logSimulationResult("GPS", simulationLog, condensedLog); // Use the new modular function
}

void runGPSMode()
{
    int speed = 100;
    int iterations = 0;

    // int speed = getValidatedIntInput("Enter movement speed (1-100, sim steps/sec): ", 1, 100);
    // int iterations = getValidatedIntInput("Enter number of iterations for the simulation (0 for infinite): ", 0, 10000);

    simulateGPSSeeking(speed, iterations);

    std::string details = "GPS Tracking\nSpeed: " + std::to_string(speed) + "\nIterations: " + std::to_string(iterations);
}

void runTestMode()
{
    if (!ensureAuditHealthy("test_mode"))
    {
        return;
    }
    if (!requirePermission("test_mode", "test_mode"))
    {
        return;
    }
    if (!logAuditOrDeny("test_mode_start", "test mode initiated", "", "test_mode"))
    {
        return;
    }
    int speed, iterations;
    std::string trackingMode;

    int stepCount = 0; // Declare stepCount outside to use in all cases

    speed = 100;
    iterations = 0;

    if (!tryGetValidatedIntInput("Enter movement speed (1-100, sim steps/sec): ", 1, 100, speed))
    {
        setUiDenialReason("input_unavailable");
        std::cout << "Recovery: verify input stream and try again.\n";
        std::cout << "Input unavailable. Returning to menu.\n";
        tools::logAuditEvent("test_mode_abort", "input unavailable", "speed");
        return;
    }
    if (!tryGetValidatedIntInput("Enter number of iterations (0 for infinite): ", 0, 10000, iterations))
    {
        setUiDenialReason("input_unavailable");
        std::cout << "Recovery: verify input stream and try again.\n";
        std::cout << "Input unavailable. Returning to menu.\n";
        tools::logAuditEvent("test_mode_abort", "input unavailable", "iterations");
        return;
    }

    const std::vector<std::string> options = {
        "Prediction",
        "Kalman Filter",
        "Heat Signature",
        "GPS",
        "Dead Reckoning"};
    const std::string help = "NOTE: YOU ARE IN TEST MODE. Use Up/Down to move, Space or Enter to select, Esc to cancel.";
    int modeChoice = tui::selectSingle("Testing and Debugging Menu", options, help);
    if (modeChoice < 0)
    {
        setUiDenialReason("selection_cancelled");
        std::cout << "Recovery: re-open the menu to select a test mode.\n";
        tools::logAuditEvent("test_mode_abort", "selection cancelled", "");
        return;
    }
    if (modeChoice >= static_cast<int>(options.size()))
    {
        setUiDenialReason("menu_selection_invalid");
        std::cerr << "Menu selection invalid. Returning to menu.\n";
        tools::logAuditEvent("test_mode_abort", "menu selection invalid", "");
        return;
    }

    switch (modeChoice + 1)
    {
    case 1:
        trackingMode = "prediction";
        break;
    case 2:
        trackingMode = "kalman";
        break;
    case 3:
        trackingMode = "heat_signature";
        break;
    case 4:
        trackingMode = "gps";
        break;
    case 5:
        trackingMode = "dead_reckoning";
        break;
    default:
        setUiDenialReason("menu_selection_invalid");
        std::cerr << "Menu selection invalid. Returning to menu.\n";
        tools::logAuditEvent("test_mode_abort", "menu selection invalid", "");
        return;
    }

    // If heat signature mode is selected, bypass manual target/follower input
    if (trackingMode == "heat_signature")
    {
        simulateHeatSeeking(speed, iterations); // Skip manual input for heat-seeking
    }
    else if (trackingMode == "kalman")
    {
        int targetX = 0;
        int targetY = 0;
        int followerX = 0;
        int followerY = 0;
        if (!tryGetValidatedIntInput("Enter initial target X position (m): ", -100000, 100000, targetX) ||
            !tryGetValidatedIntInput("Enter initial target Y position (m): ", -100000, 100000, targetY) ||
            !tryGetValidatedIntInput("Enter initial follower X position (m): ", -100000, 100000, followerX) ||
            !tryGetValidatedIntInput("Enter initial follower Y position (m): ", -100000, 100000, followerY))
        {
            setUiDenialReason("input_unavailable");
            std::cout << "Recovery: verify input stream and try again.\n";
            std::cout << "Input unavailable. Returning to menu.\n";
            tools::logAuditEvent("test_mode_abort", "input unavailable", "kalman_positions");
            return;
        }

        Object target(1, "Target", {targetX, targetY});
        Object follower(2, "Follower", {followerX, followerY});

        // Kalman filter initialization
        Tracker tracker(follower);
        tracker.setTrackingMode("kalman");
        tracker.setTarget(target);
        setUiActiveSource("kalman");

        // Run the Kalman filter tracking
        while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
        {
            tracker.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
            stepCount++;
        }
    }
    else if (trackingMode == "gps")
    {
        runGPSMode();
    }
    else if (trackingMode == "dead_reckoning")
    {
        simulateDeadReckoning(speed, iterations); // Skip manual input
    }
    else
    {
        // Get manual input for target and follower positions for non-heat-seeking modes
        int targetX = 0;
        int targetY = 0;
        int followerX = 0;
        int followerY = 0;
        if (!tryGetValidatedIntInput("Enter initial target X position (m): ", -100000, 100000, targetX) ||
            !tryGetValidatedIntInput("Enter initial target Y position (m): ", -100000, 100000, targetY) ||
            !tryGetValidatedIntInput("Enter initial follower X position (m): ", -100000, 100000, followerX) ||
            !tryGetValidatedIntInput("Enter initial follower Y position (m): ", -100000, 100000, followerY))
        {
            setUiDenialReason("input_unavailable");
            std::cout << "Recovery: verify input stream and try again.\n";
            std::cout << "Input unavailable. Returning to menu.\n";
            tools::logAuditEvent("test_mode_abort", "input unavailable", "manual_positions");
            return;
        }

        SimulationData simData = {{targetX, targetY}, {followerX, followerY}, speed, trackingMode, iterations};
        simulationHistory.push_back(simData); // Store the current simulation config

        simulateManualConfig(simData); // Call simulate with manual configuration
    }
}

// Test function for Scenario Mode with exit prompt and logging
bool runTestScenarioMode()
{
    if (!ensureAuditHealthy("test_scenario"))
    {
        return false;
    }
    if (!requirePermission("test_mode", "test_scenario"))
    {
        return false;
    }
    if (!inputStreamAvailable())
    {
        setUiDenialReason("input_unavailable");
        std::cerr << "Input stream unavailable. Exiting.\n";
        return false;
    }

    if (!logAuditOrDeny("test_scenario_start", "test scenario initiated", "", "test_scenario"))
    {
        return false;
    }
    std::cout << "Starting Test Mode for Scenario. Type 'x' then Enter to exit.\n";
    Object follower(2, "Follower", {0, 0}); // Initialize follower at origin
    int gpsTimeoutSeconds = 10;
    int heatTimeoutSeconds = 10;
    std::string logData;

    exitRequested.store(false);
    exitRequestedByUser.store(false);
    std::thread exitThread(monitorExitKey); // Non-blocking thread to monitor exit input

    setUiDecisionReason("scenario_active");
    bool completed = runScenarioMode(follower, gpsTimeoutSeconds, heatTimeoutSeconds, &exitRequested);

    // Log data handling based on exit or completion
    bool exitedByUser = exitRequestedByUser.load();
    if (!completed && !exitedByUser)
    {
        setUiDenialReason("input_unavailable");
        std::cerr << "Scenario test stopped due to input failure. Exiting.\n";
        logData += "\nTest mode stopped due to input failure.\n";
    }
    else if (exitedByUser)
    {
        tools::logAuditEvent("operator_abort", "test scenario aborted", "");
        logData += "\nTest mode exited early by user.\n";
    }
    else
    {
        tools::logAuditEvent("test_scenario_complete", "test scenario completed", "");
        logData += "\nTest mode completed normally.\n";
    }

    saveTestLog(logData);  // Save the log for later access
    exitRequested.store(true);
    exitThread.join();
    exitRequested.store(false);
    exitRequestedByUser.store(false);
    return completed || exitedByUser;
}

/****************************************
 *
 *
 * LOG RELATED FUNTIONS (VIEW, LOAD, SAVE, ETC.)
 *
 *
 *****************************************/

void viewAndRerunPreviousSimulations()
{
    if (!ensureAuditHealthy("view_history"))
    {
        return;
    }
    if (!requirePermission("simulation_history", "view_history"))
    {
        return;
    }
    if (simulationHistory.empty())
    {
        std::cout << "No previous simulations found.\n";
        return;
    }

    std::cout << "\nPrevious Simulations:\n";
    for (size_t i = 0; i < simulationHistory.size(); ++i)
    {
        const auto &sim = simulationHistory[i];
        std::cout << i + 1 << ". Target (" << sim.targetPos.first << ", " << sim.targetPos.second << "), Follower ("
                  << sim.followerPos.first << ", " << sim.followerPos.second << "), Speed: " << sim.speed
                  << ", Mode: " << sim.mode << ", Iterations: " << (sim.iterations == 0 ? "Infinite" : std::to_string(sim.iterations)) << "\n";
    }

    // Ask the user if they want to rerun any previous simulation
    int choice = 0;
    if (!tryGetValidatedIntInput("Select a simulation to rerun (0 to go back): ", 0, static_cast<int>(simulationHistory.size()), choice))
    {
        setUiDenialReason("input_unavailable");
        std::cout << "Recovery: verify input stream and try again.\n";
        std::cout << "Input unavailable. Returning to menu.\n";
        tools::logAuditEvent("history_rerun_abort", "input unavailable", "");
        return;
    }

    if (choice > 0)
    {
        if (!logAuditOrDeny("history_rerun", "rerun previous simulation", std::to_string(choice), "history_rerun"))
        {
            return;
        }
        simulateManualConfig(simulationHistory[choice - 1]); // Rerun the selected simulation
    }
}

void deletePreviousSimulation()
{
    if (!ensureAuditHealthy("delete_history"))
    {
        return;
    }
    if (!requirePermission("simulation_delete", "delete_history"))
    {
        return;
    }
    if (simulationHistory.empty())
    {
        std::cout << "No previous simulations found.\n";
        return;
    }

    std::cout << "\nPrevious Simulations:\n";
    for (size_t i = 0; i < simulationHistory.size(); ++i)
    {
        const auto &sim = simulationHistory[i];
        std::cout << i + 1 << ". Target (" << sim.targetPos.first << ", " << sim.targetPos.second << "), Follower ("
                  << sim.followerPos.first << ", " << sim.followerPos.second << "), Speed: " << sim.speed
                  << ", Mode: " << sim.mode << ", Iterations: " << (sim.iterations == 0 ? "Infinite" : std::to_string(sim.iterations)) << "\n";
    }

    int choice = 0;
    if (!tryGetValidatedIntInput("Select a simulation to delete (0 to go back): ", 0, static_cast<int>(simulationHistory.size()), choice))
    {
        setUiDenialReason("input_unavailable");
        std::cout << "Recovery: verify input stream and try again.\n";
        std::cout << "Input unavailable. Returning to menu.\n";
        tools::logAuditEvent("history_delete_abort", "input unavailable", "");
        return;
    }

    if (choice > 0)
    {
        if (!inputStreamAvailable())
        {
            setUiDenialReason("input_unavailable");
            std::cout << "Recovery: verify input stream and try again.\n";
            tools::logAuditEvent("history_delete_abort", "input unavailable", "");
            return;
        }
        std::cout << "Type DELETE to confirm: ";
        std::string confirm;
        if (!std::getline(std::cin, confirm))
        {
            setUiDenialReason("input_unavailable");
            std::cout << "Recovery: verify input stream and try again.\n";
            tools::logAuditEvent("history_delete_abort", "input unavailable", "");
            return;
        }
        if (confirm != "DELETE")
        {
            setUiDenialReason("delete_not_confirmed");
            std::cout << "Delete not confirmed. Returning to menu.\n";
            tools::logAuditEvent("history_delete_abort", "delete not confirmed", "");
            return;
        }
        // Delete the selected simulation from memory and the file
        simulationHistory.erase(simulationHistory.begin() + (choice - 1));
        saveSimulationHistoryToFile(); // Re-save the updated history after deletion

        std::cout << "Simulation deleted successfully.\n";
        if (!logAuditOrDeny("history_delete", "deleted simulation", std::to_string(choice), "history_delete"))
        {
            return;
        }
    }
    else
    {
        std::cout << "Returning to the menu.\n";
    }
}

void saveSimulationHistoryToFile()
{
    std::ofstream outFile("simulation_history.txt");
    if (!outFile)
    {
        setUiDenialReason("history_save_failed");
        std::cerr << "Error: Unable to open file for saving history.\n";
        std::cerr << "Recovery: verify write permissions and retry.\n";
        return;
    }

    for (const auto &sim : simulationHistory)
    {
        outFile << sim.targetPos.first << " " << sim.targetPos.second << " "
                << sim.followerPos.first << " " << sim.followerPos.second << " "
                << sim.speed << " " << sim.mode << " " << sim.iterations << "\n";
    }
    outFile.close();
}

// Function to save log data to a file
void saveTestLog(const std::string &logData)
{
    std::ofstream logFile("test_logs.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << logData;
        logFile << "---------------------------------------------\n";
        logFile.close();
    }
    else
    {
        setUiDenialReason("test_log_write_failed");
        std::cerr << "Error: Unable to open test logs file.\n";
        std::cerr << "Recovery: verify write permissions and retry.\n";
    }
}

void saveSimulationHistory()
{
    std::ofstream outFile("simulation_history.txt");
    if (!outFile)
    {
        setUiDenialReason("history_save_failed");
        std::cerr << "Error: Unable to open file for saving history.\n";
        std::cerr << "Recovery: verify write permissions and retry.\n";
        return;
    }

    for (const auto &sim : simulationHistory)
    {
        outFile << sim.targetPos.first << " " << sim.targetPos.second << " "
                << sim.followerPos.first << " " << sim.followerPos.second << " "
                << sim.speed << " " << sim.mode << " " << sim.iterations << "\n";
    }
    outFile.close();
}

void logSimulationResult(const std::string &mode, const std::string &details, const std::string &logDetails)
{
    std::ofstream file("simulation_history.txt", std::ios::app);
    if (file.is_open())
    {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        std::tm localTime;
        localtime_s(&localTime, &currentTime); // Use localtime_s instead of localtime

        file << "Simulation Mode: " << mode << "\n";
        file << "Time: " << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n";
        file << logDetails << "\n"; // Log a concise version for the history
        file << "---------------------------------------------\n";
        file.close();
    }
    else
    {
        setUiDenialReason("history_save_failed");
        std::cerr << "Error: Unable to open file for writing.\n";
        std::cerr << "Recovery: verify write permissions and retry.\n";
    }
}

void loadSimulationHistory()
{
    std::ifstream inFile("simulation_history.txt");
    if (!inFile)
    {
        std::cerr << "No saved simulation history found.\n";
        return;
    }

    SimulationData simData;
    while (inFile >> simData.targetPos.first >> simData.targetPos.second >>
           simData.followerPos.first >> simData.followerPos.second >>
           simData.speed >> simData.mode >> simData.iterations)
    {
        simulationHistory.push_back(simData);
    }
    inFile.close();
}


