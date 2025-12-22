#ifndef SIMULATION_H
#define SIMULATION_H

#include <utility>
#include <string>
#include <vector>
#include <atomic>
#include "ui/menu.h"

struct SimulationData
{
    std::pair<int, int> targetPos;
    std::pair<int, int> followerPos;
    int speed;
    std::string mode;
    int iterations;
};

extern std::vector<SimulationData> simulationHistory;

// Core simulation functions
void saveSimulationHistory();
void logSimulationResult(const std::string &mode, const std::string &details, const std::string &logDetails);
void loadSimulationHistory();
void simulateHeatSeeking(int speed, int iterations);
void simulateManualConfig(const SimulationData &simData);
void runGPSMode();
void runTestMode();
void runScenarioMode(Object &follower, int speed, int iterations);

// Specific functions for managing and displaying simulations
void viewAndRerunPreviousSimulations();
void deletePreviousSimulation();
void saveSimulationHistoryToFile();
void saveTestLog(const std::string &logData);

// Test mode support functions
void runTestScenarioMode();
void monitorExitKey(); // Non-blocking monitor for user input during tests

#endif // SIMULATION_H

