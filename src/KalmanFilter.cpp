#include "KalmanFilter.h"
#include <cmath>

KalmanFilter::KalmanFilter()
    : estimate({0.0, 0.0}), variance({1.0, 1.0}),
      processNoise(1e-2), measurementNoise(1e-1)
{
    // No default initialization of estimates here
}

void KalmanFilter::initialize(const std::pair<int, int> &initialPosition)
{
    estimate.first = initialPosition.first;
    estimate.second = initialPosition.second;
}

void KalmanFilter::updateEstimates(std::pair<int, int> measurement)
{
    auto [measuredX, measuredY] = measurement;

    // Kalman gain calculation
    double kalmanGainX = variance.first / (variance.first + measurementNoise);
    double kalmanGainY = variance.second / (variance.second + measurementNoise);

    // Update estimates
    estimate.first += kalmanGainX * (measuredX - estimate.first);
    estimate.second += kalmanGainY * (measuredY - estimate.second);

    // Update variances
    variance.first = (1.0 - kalmanGainX) * variance.first + processNoise;
    variance.second = (1.0 - kalmanGainY) * variance.second + processNoise;
}

std::pair<int, int> KalmanFilter::calculatePath(const Object &follower, const Object &target)
{
    // Get the target's position
    auto targetPos = target.getPosition();

    // Update estimates based on the target's position
    updateEstimates(targetPos);

    // Return the next predicted position based on current estimates
    return {static_cast<int>(std::round(estimate.first)), static_cast<int>(std::round(estimate.second))};
}
