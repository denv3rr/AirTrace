#include "Tracker.h"
#include "HeatSignature.h"
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
        pathCalculator = std::make_unique<HeatSignature>();
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

    auto followerPos = follower.getPosition3D();
    auto targetPos = target->getPosition3D();

    // GPS or other modes fallback
    if (trackingMode == "gps")
    {
        auto newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);
    }

    else if (trackingMode == "heat_signature")
    {
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2) +
                                    std::pow(targetPos.third - followerPos.third, 2));

        if (simulateHeat) // Simulate heat in test mode
        {
            float simulatedHeat = randomHeatLevel();
            updateHeatSignature(simulatedHeat);
        }
        else
        {
            float heatSignature = 100.0f / (1.0f + static_cast<float>(distance));
            updateHeatSignature(heatSignature);
        }

        if (heatSignatureData < MODE_SWITCH_THRESHOLD && ++modeSwitchCounter >= MODE_SWITCH_DELAY)
        {
            setTrackingMode("gps");
            modeSwitchCounter = 0;
            return;
        }
    }

    // Kalman filter tracking logic
    else if (trackingMode == "kalman")
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

float Tracker::randomHeatLevel()
{
    return static_cast<float>(rand() % 100);
}