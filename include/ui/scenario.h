#ifndef SCENARIO_H
#define SCENARIO_H

#include "core/Object.h"

#include <atomic>
#include <vector>
#include <string>
#include <iostream>

// Main function to run the scenario mode with GPS and heat signature
bool runScenarioMainMode(Object &follower, int gpsTimeoutSeconds, int heatTimeoutSeconds);

// Scenario execution with optional exit signal
bool runScenarioMode(Object &follower, int gpsTimeoutSeconds, int heatTimeoutSeconds, std::atomic<bool> *exitRequested);

// Helper functions for scenario mode
void logDiagnostics(const Object &follower, const std::vector<Object> &targets);

#endif // SCENARIO_H

