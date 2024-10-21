#ifndef TRACKER_H
#define TRACKER_H

#include <string>
#include <memory>
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
    void update();
    bool isTrackingActive() const;

    // Add a method to update heat signature
    void updateHeatSignature(float heatSignatureData);

private:
    Object &follower;
    std::unique_ptr<PathCalculator> pathCalculator;
    const Object *target;
    std::string trackingMode;
    bool active;
    float heatSignatureData; // To store the current heat signature
};

#endif // TRACKER_H
