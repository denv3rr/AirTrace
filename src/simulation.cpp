#include "simulation.h"
#include "Tracker.h"
#include "Object.h"
#include "inputValidation.h"
#include <iostream>
#include <thread>
#include <chrono>

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

void runTestMode()
{
    std::cout << "\n\033[32mTest Mode\033[0m\n";

    // Get user input for positions and speed
    int targetX, targetY, followerX, followerY, speed, iterations, modeChoice;
    std::string trackingMode;

    targetX = getValidatedIntInput("Enter initial target X position: ", -10000, 10000);
    targetY = getValidatedIntInput("Enter initial target Y position: ", -10000, 10000);

    followerX = getValidatedIntInput("Enter initial follower X position: ", -10000, 10000);
    followerY = getValidatedIntInput("Enter initial follower Y position: ", -10000, 10000);

    speed = getValidatedIntInput("Enter movement speed (1-10): ", 1, 10);

    // Ask if user wants to run for a fixed number of iterations or continuous mode
    std::cout << "Enter number of iterations for the simulation (0 for infinite): ";
    iterations = getValidatedIntInput("Enter number of iterations: ", 0, 1000); // 0 means infinite

    // Menu for selecting the tracking mode
    std::cout << "Choose tracking mode: \n";
    std::cout << "1. Prediction\n";
    std::cout << "2. Kalman Filter\n";
    std::cout << "3. Heat Signature\n";
    std::cout << "4. Dead Reckoning\n";
    modeChoice = getValidatedIntInput("Select a tracking mode: ", 1, 4);

    std::cout << "\n\n\n";

    // Set the tracking mode based on user input
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
        std::cout << "\033[31mInvalid choice. Defaulting to prediction mode.\033[0m\n";
        trackingMode = "prediction";
    }

    // Create objects with user-defined positions
    Object target(1, "Target", {targetX, targetY});
    Object follower(2, "Follower", {followerX, followerY});

    // Initialize tracker with manual config
    Tracker tracker(follower);
    tracker.setTrackingMode(trackingMode);
    tracker.setTarget(target);

    // Run simulation in test mode with user-defined iterations or continuous
    int stepCount = 0;
    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        tracker.update();
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed)); // Adjust based on speed
        stepCount++;
    }

    std::cout << "\n\n\nTest simulation finished.\n\n";
    std::cout << "*********************************************\n\n";
}