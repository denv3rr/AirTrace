#include "scenario.h"
#include "Tracker.h"
#include "GPSAlgorithm.h"
#include <iostream>
#include <vector>
#include <cmath>

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

float calculateHeatSignature(const Object &source, const Object &target)
{
    // Placeholder calculation based on distance (and other potential properties)
    float distance = std::sqrt(std::pow(target.getPosition().first - source.getPosition().first, 2) +
                               std::pow(target.getPosition().second - source.getPosition().second, 2));

    // Placeholder decay function inversely proportional to distance squared
    float heatSignature = source.getHeatLevel() / (1 + std::pow(distance, 2));

    return heatSignature;
}