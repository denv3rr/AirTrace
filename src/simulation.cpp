#include "simulation.h"
#include "Tracker.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>   // For sqrt, pow
#include <iomanip> // For output formatting
#include <thread>
#include <chrono>

void simulateHeatSeeking(int speed, int iterations)
{
    std::srand(static_cast<unsigned int>(std::time(0))); // Ensure casting for time seed

    // Create objects with random initial positions
    Object target(1, "Target", {std::rand() % 100, std::rand() % 100});
    Object follower(2, "Follower", {std::rand() % 100, std::rand() % 100});

    Tracker tracker(follower);
    tracker.setTrackingMode("heat_signature");
    tracker.setTarget(target);

    int stepCount = 0;

    // Simulate dynamic movement for the target in heat-seeking mode
    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        // Randomly move the target by a small amount to simulate movement
        int randomX = (std::rand() % 3) - 1; // Movement of -1, 0, or 1
        int randomY = (std::rand() % 3) - 1;
        target.moveTo({target.getPosition().first + randomX, target.getPosition().second + randomY});

        // Update heat signature data (for simplicity, let's base it on the inverse of the distance)
        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();

        // Calculate distance between follower and target
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        // Generate a heat signature reading (this could represent the heat intensity or sensor reading)
        float heatSignature = 100.0f / (1.0f + static_cast<float>(distance)); // Inverse relation to distance

        // Update the tracker with heat signature data
        tracker.updateHeatSignature(heatSignature);

        // Call the tracker update to move follower towards target
        tracker.update();

        // Display heat signature and distance data in a formatted, enterprise-quality manner
        std::cout << "\n\033[33m[Iteration " << stepCount << "]\033[0m\n";
        std::cout << "--------------------------------------------\n";
        std::cout << std::fixed << std::setprecision(2); // Format numbers with two decimal places
        std::cout << "\033[32mTarget Position: (" << targetPos.first << ", " << targetPos.second << ")\033[0m\n";
        std::cout << "\033[34mFollower Position: (" << followerPos.first << ", " << followerPos.second << ")\033[0m\n";
        std::cout << "\033[31mDistance to Target: " << distance << " units\033[0m\n";
        std::cout << "\033[36mHeat Signature: " << heatSignature << " units\033[0m\n";
        std::cout << "--------------------------------------------\n";

        // Wait for the next iteration, slowing down based on speed
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\n\033[32mHeat-seeking test simulation finished.\033[0m\n";
}
