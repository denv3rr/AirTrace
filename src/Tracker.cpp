#include "Tracker.h"
#include <iostream>

Tracker::Tracker(Object &follower) : follower(follower), target(nullptr), active(true), heatSignatureData(0.0f) {}

void Tracker::setTrackingMode(const std::string &mode)
{
    trackingMode = mode;
    if (trackingMode == "location")
    {
        pathCalculator = std::make_unique<PathCalculator>();
    }
    else if (trackingMode == "prediction")
    {
        pathCalculator = std::make_unique<PredictionAlgorithm>();
    }
    else if (trackingMode == "heat_signature")
    {
        auto heatAlgorithm = std::make_unique<HeatSignatureAlgorithm>();
        heatAlgorithm->setHeatData(heatSignatureData); // Inject heat data
        pathCalculator = std::move(heatAlgorithm);
    }
    else
    {
        std::cerr << "Unknown tracking mode: " << trackingMode << std::endl;
        active = false;
    }
}

void Tracker::setTarget(const Object &targetObj)
{
    target = &targetObj;
}

void Tracker::update()
{
    if (!target || !active)
        return;

    // Calculate new path based on current mode
    auto newPosition = pathCalculator->calculatePath(follower, *target);

    // Update the follower's position
    follower.moveTo(newPosition);

    std::cout << "Follower updated to position: " << follower.getPosition() << std::endl;
}

bool Tracker::isTrackingActive() const
{
    return active;
}
