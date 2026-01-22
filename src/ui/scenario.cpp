#include "ui/scenario.h"

#include "core/Tracker.h"
#include "core/GPSAlgorithm.h"
#include "ui/inputValidation.h"
#include "ui/simulation.h"

#include <atomic>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

bool runScenarioMode(Object &follower, int gpsTimeout, int heatTimeout, std::atomic<bool> *exitRequested)
{
    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    setUiActiveSource("gps");

    int gpsAttempts = 0;
    while (gpsAttempts++ < gpsTimeout && tracker.isTrackingActive())
    {
        if (exitRequested && exitRequested->load())
        {
            return false;
        }
        tracker.update();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (tracker.isTrackingActive())
    {
        tracker.setTrackingMode("heat_signature");
        setUiActiveSource("heat_signature");
        int heatAttempts = 0;
        while (heatAttempts++ < heatTimeout && tracker.isTrackingActive())
        {
            if (exitRequested && exitRequested->load())
            {
                return false;
            }
            tracker.update();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return true;
}

bool runScenarioMainMode(Object &follower, int gpsTimeout, int heatTimeout)
{
    if (inputStreamAvailable())
    {
        return runScenarioMode(follower, gpsTimeout, heatTimeout, nullptr);
    }
    setUiActiveSource("hold");
    setUiDenialReason("input_unavailable");
    std::cerr << "Input stream unavailable. Exiting.\n";
    return false;
}

