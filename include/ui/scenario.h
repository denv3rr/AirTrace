#ifndef SCENARIO_H
#define SCENARIO_H

#include "core/Object.h"

#include <vector>
#include <string>
#include <iostream>

// Main function to run the scenario mode with GPS and heat signature
void runScenarioMainMode(Object &follower, int gpsTimeoutSeconds, int heatTimeoutSeconds);

// Helper functions for scenario mode
void logDiagnostics(const Object &follower, const std::vector<Object> &targets);

#endif // SCENARIO_H

