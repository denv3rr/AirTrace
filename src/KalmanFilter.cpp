#include "KalmanFilter.h"

KalmanFilter::KalmanFilter()
    : estimate({0.0, 0.0}), variance({1.0, 1.0}),
      processNoise(1e-2), measurementNoise(1e-1) {}

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
    auto targetPos = target.getPosition();

    // Update estimates based on the target's position
    updateEstimates(targetPos);

    // Predict the next position based on the current estimates
    return {static_cast<int>(estimate.first), static_cast<int>(estimate.second)};
}
