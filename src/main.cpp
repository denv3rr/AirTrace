#include "../include/main.h"

int main()
{
    std::cout << "\n\033[32mStarting Tracking System...\033[0m\n\n\n";

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

    std::cout << "\n\nTracking system stopped.\n\n";
    return 0;
}
