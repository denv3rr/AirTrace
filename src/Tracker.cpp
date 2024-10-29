#include "Tracker.h"
#include "HeatSignatureAlgorithm.h"
#include <iostream>
#include <utility>
#include <cmath>

// Set these values manually if needed for testing etc
constexpr int MODE_SWITCH_THRESHOLD = 1; // Heat threshold for mode switch
constexpr int MODE_SWITCH_DELAY = 3;     // Delay in iterations before switching modes

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

// Update info
void Tracker::update()
{
    if (!target || !active)
        return;

    auto followerPos = follower.getPosition();
    auto targetPos = target->getPosition();

    // Kalman filter tracking logic
    if (trackingMode == "kalman")
    {
        auto newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);

        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));
        if (distance < 0.1)
        {
            std::cout << "\n\nFollower has reached the target at: " << follower.getPosition() << "\n";
            active = false;
        }
        return;
    }

    // Heat signature tracking logic
    else if (trackingMode == "heat_signature")
    {
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));
        float heatSignature = 100.0f / (1.0f + static_cast<float>(distance));
        updateHeatSignature(heatSignature);

        std::cout << "\033[36m[Diagnostic] Heat Signature: " << heatSignature << ", Distance: " << distance << "\033[0m\n";

        // Check if mode should switch based on heat level, with delay counter
        if (heatSignature < MODE_SWITCH_THRESHOLD)
        {
            if (++modeSwitchCounter >= MODE_SWITCH_DELAY)
            {
                std::cerr << "\033[31m[Alert] Heat signature too low. Switching to GPS mode.\033[0m\n";
                setTrackingMode("gps");
                modeSwitchCounter = 0;
                return;
            }
        }
        else
        {
            modeSwitchCounter = 0; // Reset if signature rises again
        }

        auto newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);
    }

    // GPS or other modes fallback
    else if (trackingMode == "gps")
    {
        auto newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);
    }

    if (followerPos == targetPos)
    {
        std::cout << "Follower has reached the target at: " << follower.getPosition() << "\n";
        active = false;
    }

    std::cout << "Follower updated to position: " << follower.getPosition() << "\n";
}

bool Tracker::isTrackingActive() const
{
    return active;
}

void Tracker::updateHeatSignature(float heatSignatureData)
{
    this->heatSignatureData = heatSignatureData;
}
