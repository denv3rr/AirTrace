#include "ui/scenario.h"

#include "core/Tracker.h"
#include "core/GPSAlgorithm.h"
#include "ui/inputValidation.h"
#include "ui/simulation.h"

#include <iostream>
#include <vector>
#include <cmath>

void runScenarioMode(Object &follower, int gpsTimeout, int heatTimeout)
{
    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    setUiActiveSource("gps");

    int gpsAttempts = 0;
    while (gpsAttempts++ < gpsTimeout && tracker.isTrackingActive())
    {
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
        setUiActiveSource("hold");
        setUiDenialReason("input_unavailable");
        std::cerr << "Input stream unavailable. Recovery: restart input or rerun scenario.\n";
    }
}

