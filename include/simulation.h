#ifndef SIMULATION_H
#define SIMULATION_H

#include <utility>
#include <string>
#include <vector>

#include <menu.h>

struct SimulationData
{
    std::pair<int, int> targetPos;
    std::pair<int, int> followerPos;
    int speed;
    std::string mode;
    int iterations;
};

extern std::vector<SimulationData> simulationHistory;

void saveSimulationHistory();
void logSimulationResult(const std::string &mode, const std::string &details, const std::string &logDetails);
void loadSimulationHistory();
void simulateHeatSeeking(int speed, int iterations);
void simulateManualConfig(const SimulationData &simData);
void runGPSMode();
void runTestMode();
void viewAndRerunPreviousSimulations();
void deletePreviousSimulation();
void saveSimulationHistoryToFile();

#endif // SIMULATION_H
