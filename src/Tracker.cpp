#include "Tracker.h"
#include <iostream>
#include <utility>

Tracker::Tracker(Object &follower) : follower(follower), target(nullptr), active(true), heatSignatureData(0.0f) {}

void Tracker::setTrackingMode(const std::string &mode)
{
    if (mode == "prediction")
    {
        pathCalculator = std::make_unique<PredictionAlgorithm>();
    }
    else if (mode == "kalman")
    {
        pathCalculator = std::make_unique<KalmanFilter>();
    }
    else if (mode == "heat_signature")
    {
        pathCalculator = std::make_unique<HeatSignatureAlgorithm>();
    }
    else
    {
        std::cerr << "Unknown tracking mode: " << mode << "\n";
        active = false;
    }
}

void Tracker::setTarget(const Object &targetObj)
{
    target = &targetObj;
}

void Tracker::updateHeatSignature(float heatSignatureData)
{
    this->heatSignatureData = heatSignatureData; // Store the heat signature data
}

// Update info based on heat threshold required to stop tracking
void Tracker::update()
{
    if (!target || !active)
        return;

    auto followerPos = follower.getPosition();
    auto targetPos = target->getPosition();

    // Calculate the distance between the follower and the target
    double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                std::pow(targetPos.second - followerPos.second, 2));

    if (distance < 0.1) // Threshold to "stop" once the follower reaches the target
    {
        std::cout << "Follower has reached the target at: " << follower.getPosition() << "\n";
        active = false; // Stop tracking
        return;
    }

    // Otherwise, calculate new path and update position
    auto newPosition = pathCalculator->calculatePath(follower, *target);
    follower.moveTo(newPosition);

    std::cout << "Follower updated to position: " << follower.getPosition() << "\n";
}

bool Tracker::isTrackingActive() const
{
    return active;
}
