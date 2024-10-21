#include "scenario.h"
#include "Object.h"
#include "Tracker.h"
#include <iostream>

void runRealInputScenario()
{
    std::cout << "\nStarting Real Input Scenario...\n\n";

    // Create objects
    Object target(1, "Target", {10, 10});
    Object follower(2, "Follower", {0, 0});

    // Initialize tracker
    Tracker tracker(follower);
    std::string trackingMode = "prediction"; // Can change dynamically
    tracker.setTrackingMode(trackingMode);
    tracker.setTarget(target);

    // Start tracking
    while (tracker.isTrackingActive())
    {
        tracker.update();
    }

    std::cout << "\nTracking system stopped.\n\n";
    std::cout << "*********************************************\n\n";
}
