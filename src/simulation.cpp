#include "simulation.h"
#include "Tracker.h"
#include "inputValidation.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>   // For sqrt, pow
#include <iomanip> // For output formatting
#include <thread>
#include <chrono>

// Global vector to store simulation history
std::vector<SimulationData> simulationHistory;

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

void runTestMode()
{
    // Get user input for positions and speed
    int targetX, targetY, followerX, followerY, speed, iterations, modeChoice;
    std::string trackingMode;

    targetX = getValidatedIntInput("Enter initial target X position: ", -10000, 10000);
    targetY = getValidatedIntInput("Enter initial target Y position: ", -10000, 10000);
    followerX = getValidatedIntInput("Enter initial follower X position: ", -10000, 10000);
    followerY = getValidatedIntInput("Enter initial follower Y position: ", -10000, 10000);
    speed = getValidatedIntInput("Enter movement speed (1-10): ", 1, 10);
    iterations = getValidatedIntInput("Enter number of iterations for the simulation (0 for infinite): ", 0, 1000);

    // Menu for selecting the tracking mode
    std::cout << "Choose tracking mode: \n";
    std::cout << "1. Prediction\n";
    std::cout << "2. Kalman Filter\n";
    std::cout << "3. Heat Signature\n";
    std::cout << "4. Dead Reckoning\n";
    modeChoice = getValidatedIntInput("Select a tracking mode: ", 1, 4);

    switch (modeChoice)
    {
    case 1:
        trackingMode = "prediction";
        break;
    case 2:
        trackingMode = "kalman";
        break;
    case 3:
        trackingMode = "heat_signature";
        break;
    case 4:
        trackingMode = "dead_reckoning";
        break;
    default:
        trackingMode = "prediction";
    }

    // Store the simulation data
    SimulationData simData = {{targetX, targetY}, {followerX, followerY}, speed, trackingMode, iterations};
    simulationHistory.push_back(simData); // Store the current simulation config

    // Run the simulation
    simulateManualConfig(simData);
}

void viewAndRerunPreviousSimulations()
{
    if (simulationHistory.empty())
    {
        std::cout << "\033[31mNo previous simulations found.\033[0m\n";
        return;
    }

    std::cout << "\n\033[32mPrevious Simulations:\033[0m\n";
    for (size_t i = 0; i < simulationHistory.size(); ++i)
    {
        const auto &sim = simulationHistory[i];
        std::cout << i + 1 << ". Target (" << sim.targetPos.first << ", " << sim.targetPos.second << "), Follower ("
                  << sim.followerPos.first << ", " << sim.followerPos.second << "), Speed: " << sim.speed
                  << ", Mode: " << sim.mode << ", Iterations: " << (sim.iterations == 0 ? "Infinite" : std::to_string(sim.iterations)) << "\n";
    }

    // Ask the user if they want to rerun any previous simulation
    int choice = getValidatedIntInput("Select a simulation to rerun (0 to go back): ", 0, simulationHistory.size());

    if (choice > 0)
    {
        simulateManualConfig(simulationHistory[choice - 1]); // Rerun the selected simulation
    }
}

void simulateManualConfig(const SimulationData &simData)
{
    // Use the simData to run the simulation
    Object target(1, "Target", simData.targetPos);
    Object follower(2, "Follower", simData.followerPos);

    Tracker tracker(follower);
    tracker.setTrackingMode(simData.mode);
    tracker.setTarget(target);

    int stepCount = 0;
    while (tracker.isTrackingActive() && (simData.iterations == 0 || stepCount < simData.iterations))
    {
        tracker.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / simData.speed)); // Adjust based on speed
        stepCount++;
    }

    std::cout << "\n\nTest simulation finished.\n\n";
}
