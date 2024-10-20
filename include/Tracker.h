#ifndef TRACKER_H
#define TRACKER_H

#include <string>
#include <memory>
#include "Object.h"
#include "PathCalculator.h"
#include "PredictionAlgorithm.h"
#include "HeatSignatureAlgorithm.h"

class Tracker
{
public:
    Tracker(Object &follower);
    void setTrackingMode(const std::string &mode);
    void setTarget(const Object &target);
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

#endif // TRACKER_H
