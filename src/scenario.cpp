#include "scenario.h"
#include "Tracker.h"
#include "GPSAlgorithm.h"
#include <iostream>
#include <vector>

// Main Scenario Mode
void runScenarioMode(Object &follower, int speed, int iterations)
{
    if (follower.getPosition() == std::pair<int, int>({0, 0}))
    {
        std::cerr << "\033[31mNo valid GPS data or input stream. Scenario Mode halted.\033[0m\n";
        return;
    }

    std::vector<Object> targets = generateTargets(); // Hypothetical helper to generate targets
    Tracker tracker(follower);
    tracker.setTrackingMode("gps"); // Start in GPS mode
    tracker.setTarget(targets[0]);

    tracker.startTracking(iterations, speed);

    // Switch to heat signature tracking
    std::cout << "\n\033[32mSwitching to Heat Signature Tracking Mode\033[0m\n";
    tracker.setTrackingMode("heat_signature");

    for (int i = 0; tracker.isTrackingActive() && i < iterations; ++i)
    {
        for (const auto &target : targets)
        {
            float heatSignature = calculateHeatSignature(follower, target); // Function to calculate heat
            tracker.updateHeatSignature(heatSignature);
            tracker.update();
        }

        logDiagnostics(follower, targets); // Log diagnostics for review
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
    }
}