#ifndef SIMULATION_H
#define SIMULATION_H

#include <utility>
#include <string>
#include <vector>

struct SimulationData
{
    std::pair<int, int> targetPos;
    std::pair<int, int> followerPos;
    int speed;
    std::string mode;
    int iterations;
};

extern std::vector<SimulationData> simulationHistory;

void simulateHeatSeeking(int speed, int iterations); // Add this declaration
void simulateManualConfig(const SimulationData &simData);
void runTestMode();
void viewAndRerunPreviousSimulations();

#endif // SIMULATION_H
