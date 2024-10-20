#include "Tracker.h"
#include <iostream>
#include <utility>

// Overload << for std::pair
template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &p)
{
    os << "(" << p.first << ", " << p.second << ")";
    return os;
}

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
        std::cerr << "\n\nUnknown tracking mode: " << mode << "\n\n";
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

    // Get the current position of the follower and the target
    auto followerPos = follower.getPosition();
    auto targetPos = target->getPosition();

    // Check if the follower is "close enough" to the target
    int threshold = 1; // Set a threshold for proximity (can be adjusted)
    if (std::abs(followerPos.first - targetPos.first) <= threshold &&
        std::abs(followerPos.second - targetPos.second) <= threshold)
    {
        std::cout << "\n\033[32mFollower has reached the target at: " << followerPos << "\033[0m\n\n\n";
        active = false; // Stop tracking once the target is reached
        return;
    }

    // Otherwise, calculate new path and update position
    auto newPosition = pathCalculator->calculatePath(follower, *target);
    follower.moveTo(newPosition);

    std::cout << "\033[32mFollower updated to position: " << follower.getPosition() << "\033[0m\n";
}

bool Tracker::isTrackingActive() const
{
    return active;
}
