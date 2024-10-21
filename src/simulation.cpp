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
    std::srand(static_cast<unsigned int>(std::time(0))); // Seed for random movements

    // Initialize random positions for target and follower
    Object target(1, "Target", {std::rand() % 100, std::rand() % 100});
    Object follower(2, "Follower", {std::rand() % 100, std::rand() % 100});

    Tracker tracker(follower);
    tracker.setTrackingMode("heat_signature");
    tracker.setTarget(target);

    int stepCount = 0;

    // Dynamically move target and update heat signature
    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        // Random movement for the target simulating real-world data
        int randomX = (std::rand() % 3) - 1; // -1, 0, or 1
        int randomY = (std::rand() % 3) - 1;
        target.moveTo({target.getPosition().first + randomX, target.getPosition().second + randomY});

        // Calculate distance between target and follower
        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        // Simulate a heat signature, stronger the closer the follower is
        float heatSignature = 100.0f / (1.0f + static_cast<float>(distance));

        // Update tracker with the heat signature data
        tracker.updateHeatSignature(heatSignature);
        tracker.update();

        std::cout << "\n\033[33m[Iteration " << stepCount << "]\033[0m\n";
        std::cout << "--------------------------------------------\n";
        std::cout << std::fixed << std::setprecision(2); // Two decimal places for numbers
        std::cout << "\033[32mTarget Position: (" << targetPos.first << ", " << targetPos.second << ")\033[0m\n";
        std::cout << "\033[34mFollower Position: (" << followerPos.first << ", " << followerPos.second << ")\033[0m\n";
        std::cout << "\033[31mDistance to Target: " << distance << " units\033[0m\n";
        std::cout << "\033[36mHeat Signature: " << heatSignature << " units\033[0m\n";
        std::cout << "--------------------------------------------\n";

        // If distance is extremely small, stop the simulation (the follower "reaches" the target)
        if (distance < 0.1)
        {
            std::cout << "\033[32mFollower has crashed into the target.\033[0m\n";
            break;
        }

        // Sleep based on speed
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\n\033[32mHeat-seeking simulation finished.\033[0m\n";
}

void runTestMode()
{
    int speed, iterations, modeChoice, targetX, targetY, followerX, followerY;
    std::string trackingMode;

    int stepCount = 0; // Declare stepCount outside to use in all cases

    // General user input for speed and iterations
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
        trackingMode = "heat_signature"; // Heat-seeking shouldn't require manual input
        break;
    case 4:
        trackingMode = "dead_reckoning";
        break;
    default:
        std::cout << "\033[31mInvalid choice. Defaulting to prediction mode.\033[0m\n";
        trackingMode = "prediction";
    }

    // If heat signature mode is selected, bypass manual target/follower input
    if (trackingMode == "heat_signature")
    {
        simulateHeatSeeking(speed, iterations); // Skip manual input for heat-seeking
    }
    else
    {
        // Get manual input for target and follower positions for non-heat-seeking modes
        int targetX = getValidatedIntInput("Enter initial target X position: ", -10000, 10000);
        int targetY = getValidatedIntInput("Enter initial target Y position: ", -10000, 10000);
        int followerX = getValidatedIntInput("Enter initial follower X position: ", -10000, 10000);
        int followerY = getValidatedIntInput("Enter initial follower Y position: ", -10000, 10000);

        SimulationData simData = {{targetX, targetY}, {followerX, followerY}, speed, trackingMode, iterations};
        simulationHistory.push_back(simData); // Store the current simulation config

        simulateManualConfig(simData); // Call simulate with manual configuration
    }

    if (trackingMode == "kalman")
    {
        // Make sure the initial positions are set correctly here
        Object target(1, "Target", {targetX, targetY});
        Object follower(2, "Follower", {followerX, followerY});

        // Kalman filter initialization
        Tracker tracker(follower);
        tracker.setTrackingMode("kalman");
        tracker.setTarget(target);

        // Run the Kalman filter tracking
        while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
        {
            tracker.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
            stepCount++;
        }
    }
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
    int choice = getValidatedIntInput("Select a simulation to rerun (0 to go back): ", 0, static_cast<int>(simulationHistory.size()));

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
