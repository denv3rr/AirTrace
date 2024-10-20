#include "main.h"

int main()
{
    std::cout << "Starting Tracking System...\n";

    // Create objects
    Object target(1, "Target", {10, 10});
    Object follower(2, "Follower", {0, 0});

    // Initialize tracker
    Tracker tracker(follower);

    // Select mode: Heat signature tracking, location, prediction
    std::string trackingMode = "prediction"; // This can be dynamically changed at runtime
    tracker.setTrackingMode(trackingMode);

    // Set the target to track
    tracker.setTarget(target);

    // Start tracking
    while (tracker.isTrackingActive())
    {
        tracker.update();
    }

    std::cout << "Tracking system stopped.\n";
    return 0;
}
