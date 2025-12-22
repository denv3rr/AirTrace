#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

#include "core/PathCalculator.h"
#include <utility> // For std::pair
#include <iostream>

class KalmanFilter : public PathCalculator
{
public:
    KalmanFilter();                                                                           // Constructor
    void initialize(const std::pair<int, int> &initialPosition);                              // Method to initialize with follower's starting position
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override; // Override calculatePath

private:
    std::pair<double, double> estimate; // Estimate position (x, y)
    std::pair<double, double> variance; // Variance for Kalman filter
    double processNoise;
    double measurementNoise;

    void updateEstimates(std::pair<int, int> measurement); // Update estimates based on measurement
};

#endif // KALMAN_FILTER_H

