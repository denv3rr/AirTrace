#ifndef SCENARIO_H
#define SCENARIO_H

#include "Object.h"

#include <vector>
#include <string>
#include <iostream>

// Main function to run the scenario mode with GPS and heat signature
void runScenarioMainMode(Object &follower, int speed, int iterations);

// Helper functions for scenario mode
std::vector<Object> generateTargets();
float calculateHeatSignature(const Object &follower, const Object &target);
void logDiagnostics(const Object &follower, const std::vector<Object> &targets);

#endif // SCENARIO_H
