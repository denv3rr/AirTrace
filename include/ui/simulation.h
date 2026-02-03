#ifndef SIMULATION_H
#define SIMULATION_H

#include <utility>
#include <string>
#include <vector>
#include <atomic>

#include "core/Object.h"
#include "core/multi_modal_types.h"
#include "core/sim_config.h"

struct SimulationData
{
    std::pair<int, int> targetPos;
    std::pair<int, int> followerPos;
    int speed;
    std::string mode;
    int iterations;
};

extern std::vector<SimulationData> simulationHistory;

struct UiStatus
{
    std::string platformProfile;
    std::string parentProfile;
    std::string childModules;
    std::string activeSource;
    std::string contributors;
    double modeConfidence = 0.0;
    std::string concurrencyStatus;
    std::string decisionReason;
    std::string denialReason;
    std::string disqualifiedSources;
    std::string lockoutStatus;
    std::string ladderStatus;
    std::string sensorStatusSummary;
    std::string authStatus;
    std::string provenanceStatus;
    std::string loggingStatus;
    std::string adapterId;
    std::string adapterVersion;
    std::string adapterSurface;
    std::string adapterStatus;
    std::string adapterFields;
    unsigned int seed = 0;
    bool deterministic = true;
};

struct SensorUiSnapshot
{
    std::string name;
    bool available = true;
    bool healthy = true;
    bool hasMeasurement = false;
    double timeSinceLastValid = 0.0;
    double confidence = 0.0;
    std::string lastError;
};

bool initializeUiContext(const std::string &configPath);
const UiStatus &getUiStatus();
void setUiActiveSource(const std::string &source);
void setUiContributors(const std::vector<std::string> &contributors);
void setUiModeConfidence(double confidence);
void setUiConcurrencyStatus(const std::string &status);
void setUiDecisionReason(const std::string &reason);
void setUiDenialReason(const std::string &reason);
void setUiDisqualifiedSources(const std::string &summary);
void setUiLockoutStatus(const std::string &summary);
void setUiLadderStatus(const std::string &summary);
void setUiSensorStatusSummary(const std::string &summary);
void setUiLoggingStatus(const std::string &status);
void updateUiFromModeDecision(const ModeDecisionDetail &detail, const std::vector<SensorUiSnapshot> &sensors);
bool uiEnsureAuditHealthy(const std::string &context);
bool uiHasPermission(const std::string &permission);
void uiRenderStatusBanner(const std::string &context);
void resetUiRng();
int uiRandomInt(int minValue, int maxValue);

// Core simulation functions
void saveSimulationHistory();
void logSimulationResult(const std::string &mode, const std::string &details, const std::string &logDetails);
void loadSimulationHistory();
void simulateHeatSeeking(int speed, int iterations);
void simulateManualConfig(const SimulationData &simData);
void runGPSMode();
void runTestMode();

// Specific functions for managing and displaying simulations
void viewAndRerunPreviousSimulations();
void deletePreviousSimulation();
void saveSimulationHistoryToFile();
void saveTestLog(const std::string &logData);

// Test mode support functions
bool runTestScenarioMode();
void monitorExitKey(); // Non-blocking monitor for user input during tests

#endif // SIMULATION_H

