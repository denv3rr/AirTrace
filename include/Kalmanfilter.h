#ifndef KALMANFILTER_H
#define KALMANFILTER_H

#include "PathCalculator.h"
#include <utility>

class KalmanFilter : public PathCalculator
{
public:
    KalmanFilter();
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;
    void updateEstimates(std::pair<int, int> measurement);

private:
    std::pair<double, double> estimate;
    std::pair<double, double> variance;
    double processNoise;
    double measurementNoise;
};

#endif // KALMANFILTER_H
