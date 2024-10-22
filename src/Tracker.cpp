#include "Tracker.h"
#include "HeatSignatureAlgorithm.h"
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
    else if (mode == "gps")
    {
        pathCalculator = std::make_unique<GPSAlgorithm>(); // Assuming GPSAlgorithm exists
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

void Tracker::startTracking(int iterations, int speed)
{
    int stepCount = 0;
    while (isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }
}

// Update info based on heat threshold required to stop tracking
void Tracker::update()
{
    if (!target || !active)
        return;

    auto followerPos = follower.getPosition();
    auto targetPos = target->getPosition();

    // Kalman filter tracking logic
    if (trackingMode == "kalman")
    {
        auto newPosition = pathCalculator->calculatePath(follower, *target); // Prediction step
        follower.moveTo(newPosition);                                        // Move based on Kalman prediction

        // If the distance is small enough, stop
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));
        if (distance < 0.1)
        {
            std::cout << "\n\n\nFollower has reached the target at: " << follower.getPosition() << "\n";
            active = false; // Stop tracking
        }
        return;
    }

    // Heat signature tracking logic
    else if (trackingMode == "heat_signature" && heatSignatureData < 1 /*< THRESHOLD*/)
    {
        std::cerr << "Heat sensor failure detected. Attempting to use GPS.\n";
        setTrackingMode("gps");
    }

    // GPS fallback if heat sensor fails
    else if (followerPos == targetPos)
    {
        std::cout << "Follower has reached the target at: " << follower.getPosition() << "\n";
        active = false;
        return;
    }

    // Regular path update
    auto newPosition = pathCalculator->calculatePath(follower, *target);
    follower.moveTo(newPosition);
    std::cout << "Follower updated to position: " << follower.getPosition() << "\n";
}

bool Tracker::isTrackingActive() const
{
    return active;
}
