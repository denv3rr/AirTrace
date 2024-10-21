#ifndef SIMULATION_H
#define SIMULATION_H

#include <string>

// Function to simulate heat-seeking mode with dynamic target movements
void simulateHeatSeeking(int speed, int iterations);

// Function to simulate manually configured modes (e.g., prediction, Kalman, dead reckoning)
void simulateManualConfig(const std::string &trackingMode, int speed, int iterations);

#endif // SIMULATION_H
