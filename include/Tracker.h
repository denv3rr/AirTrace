#ifndef TRACKER_H
#define TRACKER_H

#include <string>
#include <memory>
#include <utility>
#include <iostream>
#include <cmath>
#include <chrono>
#include <thread>

#include "Object.h"
#include "PathCalculator.h"
#include "PredictionAlgorithm.h"
#include "KalmanFilter.h"
#include "HeatSignatureAlgorithm.h"
#include "DeadReckoning.h"

class Tracker
{
public:
    Tracker(Object &follower);
    void setTrackingMode(const std::string &mode);
    void setTarget(const Object &target);
    void startTracking(int iterations, int speed);
    void updateHeatSignature(float heatSignatureData);
    void update();
    bool isTrackingActive() const;

private:
    Object &follower;
    std::unique_ptr<PathCalculator> pathCalculator;
    const Object *target;
    std::string trackingMode;
    bool active;
    float heatSignatureData;
};

// Make sure the function template is inline to avoid redefinition
template <typename T1, typename T2>
inline std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p)
{
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

#endif // TRACKER_H
