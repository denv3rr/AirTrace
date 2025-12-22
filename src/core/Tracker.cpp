#include "core/Tracker.h"
#include "core/HeatSignature.h"
#include <iostream>
#include <utility>
#include <cmath>

// Set these values manually if needed for testing etc
constexpr int MODE_SWITCH_THRESHOLD = 1; // Heat threshold for mode switch
constexpr int MODE_SWITCH_DELAY = 3;     // Delay in iterations before switching modes

Tracker::Tracker(Object &follower)
    : modeSwitchCounter(0), follower(follower), target(nullptr), active(true), heatSignatureData(0.0f) {}

void Tracker::setTrackingMode(const std::string &mode)
{
    trackingMode = mode;
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
    else if (mode == "dead_reckoning")
    {
        pathCalculator = std::make_unique<DeadReckoning>();
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

    if (!pathCalculator)
    {
        active = false;
        return;
    }

    auto followerPos = follower.getPosition();
    auto targetPos = target->getPosition();
    std::pair<int, int> newPosition = followerPos;

    if (trackingMode == "heat_signature")
    {
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        float heatSignature = heatSignatureData;
        if (heatSignature <= 0.0f)
        {
            heatSignature = 100.0f / (1.0f + static_cast<float>(distance));
        }

        if (auto *heatCalc = dynamic_cast<HeatSignature *>(pathCalculator.get()))
        {
            heatCalc->setHeatData(heatSignature);
        }

        updateHeatSignature(heatSignature);

        newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);

        if (heatSignatureData < MODE_SWITCH_THRESHOLD && ++modeSwitchCounter >= MODE_SWITCH_DELAY)
        {
            setTrackingMode("gps");
            modeSwitchCounter = 0;
        }
    }
    else
    {
        newPosition = pathCalculator->calculatePath(follower, *target);
        follower.moveTo(newPosition);
    }

    followerPos = follower.getPosition();
    targetPos = target->getPosition();

    double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                std::pow(targetPos.second - followerPos.second, 2));
    if (distance < 0.1)
    {
        std::cout << "Follower has reached the target at: " << follower.getPosition() << "\n";
        active = false;
        return;
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

