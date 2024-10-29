#include "scenario.h"
#include "Tracker.h"
#include "GPSAlgorithm.h"
#include <iostream>
#include <vector>
#include <cmath>

void runScenarioMode(Object &follower, int gpsTimeout, int heatTimeout)
{
    Tracker tracker(follower);
    tracker.setTrackingMode("gps");

    int gpsAttempts = 0;
    while (gpsAttempts++ < gpsTimeout && tracker.isTrackingActive())
    {
        tracker.update();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (tracker.isTrackingActive())
    {
        tracker.setTrackingMode("heat_signature");
        int heatAttempts = 0;
        while (heatAttempts++ < heatTimeout && tracker.isTrackingActive())
        {
            tracker.update();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }
}

void runScenarioMainMode(Object &follower, int gpsTimeout, int heatTimeout)
{
    if (inputStreamAvailable())
    {
        runScenarioMode(follower, gpsTimeout, heatTimeout);
    }
    else
    {
        std::cerr << "Input stream unavailable. Returning to main menu.\n";
    }
}

float calculateHeatSignature(const Object &source, const Object &target)
{
    // Placeholder calculation based on distance (and other potential properties)
    float distance = std::sqrt(std::pow(target.getPosition().first - source.getPosition().first, 2) +
                               std::pow(target.getPosition().second - source.getPosition().second, 2));

    // Placeholder decay function inversely proportional to distance squared
    float heatSignature = source.getHeatLevel() / (1 + std::pow(distance, 2));

    return heatSignature;
}