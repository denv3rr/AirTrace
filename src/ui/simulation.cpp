// This file holds test functionality for different modes

#include "ui/simulation.h"
#include "core/Tracker.h"
#include "ui/inputValidation.h"
#include "ui/scenario.h"
#include "ui/tui.h"
#include "ui/alerts.h"
#include "ui/adapter_ui_mapping.h"
#include "tools/audit_log.h"
#include "tools/adapter_registry_loader.h"
#include "tools/sim_config_loader.h"
#include <iostream>
#include <cmath>
#include <atomic>
#include <iomanip>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <random>
#include <cctype>
#include <algorithm>
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
};

UiContext uiContext{};

bool hasPermission(const std::string &permission)
{
#if defined(AIRTRACE_TEST_HARNESS)
    if (permission == "test_mode" || permission == "simulation_history" || permission == "simulation_delete")
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
    uiContext.status.concurrencyStatus = "none";
    uiContext.status.decisionReason = "";
    uiContext.status.denialReason = "";
    if (uiContext.status.activeSource.empty())
    {
        uiContext.status.activeSource = "none";
    }
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
        uiContext.status.platformProfile = "base";
        uiContext.status.parentProfile = "none";
        uiContext.status.childModules = "none";
        uiContext.status.authStatus = "config_invalid";
        uiContext.status.denialReason = "config_invalid";
        uiContext.status.seed = uiContext.seed;
        uiContext.status.deterministic = true;
        if (uiContext.status.activeSource.empty())
        {
            uiContext.status.activeSource = "none";
        }
        return false;
    }

    uiContext.config = loaded.config;
    uiContext.configLoaded = true;
    uiContext.seed = loaded.config.seed;
    uiContext.rng.seed(uiContext.seed);
    updateStatusFromConfig(loaded.config);
    {
        tools::AdapterUiSnapshot snapshot = tools::loadAdapterUiSnapshot(loaded.config);
        uiContext.status.adapterId = snapshot.adapterId;
        uiContext.status.adapterVersion = snapshot.adapterVersion;
        uiContext.status.adapterSurface = snapshot.surface.empty() ? "tui" : snapshot.surface;
        uiContext.status.adapterStatus = snapshot.status;
        uiContext.status.adapterFields = ui::formatAdapterFieldSummary(snapshot.fields);
    }
    tools::setAuditRunContext("", loaded.config.version, loaded.config.seed);
    return true;
}

const UiStatus &getUiStatus()
{
    return uiContext.status;
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


