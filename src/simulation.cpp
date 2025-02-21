// This file holds test functionality for different modes

#include "simulation.h"
#include "Tracker.h"
#include "inputValidation.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <atomic>
#include <iomanip>
#include <thread>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>

// Global vector to store simulation history
std::vector<SimulationData> simulationHistory;

// Atomic variable to allow exit during test mode
std::atomic<bool> exitRequested(false);

/****************************************
 *
 *
 * PROGRAM/INTERACTIVITY FUNCTIONS
 *
 *
 *****************************************/

// Monitor function to check for user input without blocking
void monitorExitKey()
{
    std::string input;
    while (!exitRequested.load())
    { // Use load() for atomic
        std::cin >> input;
        if (input == "x")
        {
            exitRequested.store(true); // Use store() for atomic
            std::cout << "\033[31m\nExiting to test menu...\033[0m\n";
        }
    }
}

/****************************************
 *
 *
 * SIMULATION FUNCTIONS
 *
 *
 *****************************************/

std::vector<Object> generateTargets()
{
    std::vector<Object> targets;

    // Example: Generating 5 targets with sample positions
    for (int i = 0; i < 5; ++i)
    {
        Object target(i + 1, "Target_" + std::to_string(i + 1), {rand() % 100, rand() % 100});
        targets.push_back(target);
    }

    return targets;
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

void simulateDeadReckoning(int speed, int iterations)
{
    Object target(1, "Target", {std::rand() % 100, std::rand() % 100});
    Object follower(2, "Follower", {std::rand() % 100, std::rand() % 100});

    Tracker tracker(follower);
    tracker.setTrackingMode("dead_reckoning");
    tracker.setTarget(target);

    int stepCount = 0;
    std::string simulationLog;
    std::string condensedLog;

    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        tracker.update();

        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        std::cout << "\033[33m[Iteration " << stepCount << "] Dead Reckoning Mode\033[0m\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "\033[32mTarget Position: (" << targetPos.first << ", " << targetPos.second << ")\033[0m\n";
        std::cout << "\033[34mFollower Position: (" << followerPos.first << ", " << followerPos.second << ")\033[0m\n";
        std::cout << "\033[31mDistance to Target: " << distance << " units\033[0m\n";
        std::cout << "--------------------------------------------\n";

        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) + "\n";
        condensedLog += "Iteration " + std::to_string(stepCount) + " - Distance: " + std::to_string(distance) + " units\n";

        if (distance < 0.1)
        {
            std::cout << "\n\033[32mFollower has reached the target.\033[0m\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\n\033[32mDead Reckoning simulation finished.\033[0m\n";
    logSimulationResult("Dead Reckoning", simulationLog, condensedLog);
}

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
    std::string simulationLog;
    std::string condensedLog;

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

        // Simulate a heat signature for test, stronger the closer the follower is
        float heatSignature = 100.0f / (1.0f + static_cast<float>(distance));

        // Update tracker with the heat signature data
        tracker.updateHeatSignature(heatSignature);
        tracker.update();

        // Detailed output for the user
        std::cout << "\n\033[33m[Iteration " << stepCount << "]\033[0m\n";
        std::cout << "--------------------------------------------\n";
        std::cout << std::fixed << std::setprecision(2); // Two decimal places for numbers
        std::cout << "\033[32mTarget Position: (" << targetPos.first << ", " << targetPos.second << ")\033[0m\n";
        std::cout << "\033[34mFollower Position: (" << followerPos.first << ", " << followerPos.second << ")\033[0m\n";
        std::cout << "\033[31mDistance to Target: " << distance << " units\033[0m\n";
        std::cout << "\033[36mHeat Signature: " << heatSignature << " units\033[0m\n";
        std::cout << "--------------------------------------------\n";

        // Logging compact information for the text file
        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) +
                         ", Heat Signature: " + std::to_string(heatSignature) + "\n";

        // If distance is extremely small, stop the simulation (the follower "reaches" the target)
        if (distance < 0.1)
        {
            std::cout << "\n\033[32mFollower has hit the target and stopped.\033[0m\n";
            break;
        }

        // Sleep based on speed
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\n\033[32mHeat-seeking simulation finished.\033[0m\n\n--------------------------------------------\n\n";
    logSimulationResult("Heat Seeking", simulationLog, condensedLog); // Log simulation details
}

void simulateGPSSeeking(int speed, int iterations)
{
    Object target(1, "Target", {std::rand() % 100, std::rand() % 100});
    Object follower(2, "Follower", {std::rand() % 100, std::rand() % 100});

    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    tracker.setTarget(target);

    int stepCount = 0;
    std::string simulationLog;
    std::string condensedLog;

    while (tracker.isTrackingActive() && (iterations == 0 || stepCount < iterations))
    {
        int randomX = (std::rand() % 3) - 1;
        int randomY = (std::rand() % 3) - 1;
        target.moveTo({target.getPosition().first + randomX, target.getPosition().second + randomY});

        tracker.update();

        auto targetPos = target.getPosition();
        auto followerPos = follower.getPosition();
        double distance = std::sqrt(std::pow(targetPos.first - followerPos.first, 2) +
                                    std::pow(targetPos.second - followerPos.second, 2));

        std::cout << "\033[33m[Iteration " << stepCount << "] GPS Mode\033[0m\n";
        std::cout << "--------------------------------------------\n";
        std::cout << "\033[32mTarget GPS Position: (" << targetPos.first << ", " << targetPos.second << ")\033[0m\n";
        std::cout << "\033[34mFollower GPS Position: (" << followerPos.first << ", " << followerPos.second << ")\033[0m\n";
        std::cout << "\033[31mDistance to Target: " << distance << " units\033[0m\n";
        std::cout << "--------------------------------------------\n";

        simulationLog += "Iteration: " + std::to_string(stepCount) + ", Distance: " + std::to_string(distance) + "\n";
        condensedLog += "Iteration " + std::to_string(stepCount) + " - Distance: " + std::to_string(distance) + " units\n";

        if (distance < 0.1)
        {
            std::cout << "\n\033[32mFollower has reached the target.\033[0m\n";
            break;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed));
        stepCount++;
    }

    std::cout << "\033[32mGPS-based simulation finished.\033[0m\n";
    logSimulationResult("GPS", simulationLog, condensedLog); // Use the new modular function
}

void runGPSMode()
{
    int speed = 100;
    int iterations = 0;

    // int speed = getValidatedIntInput("Enter movement speed (1-100): ", 1, 100);
    // int iterations = getValidatedIntInput("Enter number of iterations for the simulation (0 for infinite): ", 0, 10000);

    simulateGPSSeeking(speed, iterations);

    std::string details = "GPS Tracking\nSpeed: " + std::to_string(speed) + "\nIterations: " + std::to_string(iterations);
}

void runTestMode()
{
    int speed, iterations, modeChoice, targetX, targetY, followerX, followerY;
    std::string trackingMode;

    int stepCount = 0; // Declare stepCount outside to use in all cases

    speed = 100;
    iterations = 0;

    // General manual user input for speed and iterations
    // speed = getValidatedIntInput("Enter movement speed (1-100): ", 1, 100);
    // iterations = getValidatedIntInput("Enter number of iterations for the simulation (0 for infinite): ", 0, 10000);

    // Menu for selecting the tracking mode
    std::cout << "\n\n--------------------------------------------\n";
    std::cout << "\n\nTesting and Debugging Menu\n\n\033[33mNOTE: YOU ARE IN 'TEST MODE'\nThe 'scenario mode' is being designed for dynamic switching\nbetween these modes automatically based on external input.\033[0m\n\n";
    std::cout << "1. Prediction\n";
    std::cout << "2. Kalman Filter\n";
    std::cout << "3. Heat Signature\n";
    std::cout << "4. GPS\n";
    std::cout << "5. Dead Reckoning\n\n";
    modeChoice = getValidatedIntInput("Select a tracking mode: ", 1, 5);
    std::cout << "\n\n--------------------------------------------\n\n";

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
        trackingMode = "gps";
        break;
    case 5:
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
    else if (trackingMode == "kalman")
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
    else if (trackingMode == "gps")
    {
        runGPSMode();
    }
    else if (trackingMode == "dead_reckoning")
    {
        simulateDeadReckoning(speed, iterations); // Skip manual input
    }
    else
    {
        // Get manual input for target and follower positions for non-heat-seeking modes
        int targetX = getValidatedIntInput("Enter initial target X position: ", -100000, 100000);
        int targetY = getValidatedIntInput("Enter initial target Y position: ", -100000, 100000);
        int followerX = getValidatedIntInput("Enter initial follower X position: ", -100000, 10000);
        int followerY = getValidatedIntInput("Enter initial follower Y position: ", -100000, 100000);

        SimulationData simData = {{targetX, targetY}, {followerX, followerY}, speed, trackingMode, iterations};
        simulationHistory.push_back(simData); // Store the current simulation config

        simulateManualConfig(simData); // Call simulate with manual configuration
    }
}

// Test function for Scenario Mode with exit prompt and logging
void runTestScenarioMode()
{
    std::cout << "\033[32mStarting Test Mode for Scenario. Press 'x' to exit.\033[0m\n";
    Object follower(2, "Follower", {0, 0}); // Initialize follower at origin
    int speed = 100;
    int iterations = 100;
    std::string logData;

    std::thread exitThread(monitorExitKey); // Non-blocking thread to monitor exit input

    runScenarioMode(follower, speed, iterations); // Call main scenario function

    // Log data handling based on exit or completion
    if (exitRequested)
    {
        logData += "\nTest mode exited early by user.\n";
    }
    else
    {
        logData += "\nTest mode completed normally.\n";
    }

    saveTestLog(logData);  // Save the log for later access
    exitRequested = false; // Reset for future runs
    exitThread.join();
}

/****************************************
 *
 *
 * LOG RELATED FUNTIONS (VIEW, LOAD, SAVE, ETC.)
 *
 *
 *****************************************/

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

void deletePreviousSimulation()
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

    int choice = getValidatedIntInput("Select a simulation to delete (0 to go back): ", 0, static_cast<int>(simulationHistory.size()));

    if (choice > 0)
    {
        // Delete the selected simulation from memory and the file
        simulationHistory.erase(simulationHistory.begin() + (choice - 1));
        saveSimulationHistoryToFile(); // Re-save the updated history after deletion

        std::cout << "\033[32mSimulation deleted successfully.\033[0m\n";
    }
    else
    {
        std::cout << "\033[33mReturning to the menu.\033[0m\n";
    }
}

void saveSimulationHistoryToFile()
{
    std::ofstream outFile("simulation_history.txt");
    if (!outFile)
    {
        std::cerr << "Error: Unable to open file for saving history.\n";
        return;
    }

    for (const auto &sim : simulationHistory)
    {
        outFile << sim.targetPos.first << " " << sim.targetPos.second << " "
                << sim.followerPos.first << " " << sim.followerPos.second << " "
                << sim.speed << " " << sim.mode << " " << sim.iterations << "\n";
    }
    outFile.close();
}

// Function to save log data to a file
void saveTestLog(const std::string &logData)
{
    std::ofstream logFile("test_logs.txt", std::ios::app);
    if (logFile.is_open())
    {
        logFile << logData;
        logFile << "---------------------------------------------\n";
        logFile.close();
    }
    else
    {
        std::cerr << "Error: Unable to open test logs file.\n";
    }
}

void saveSimulationHistory()
{
    std::ofstream outFile("simulation_history.txt");
    if (!outFile)
    {
        std::cerr << "Error: Unable to open file for saving history.\n";
        return;
    }

    for (const auto &sim : simulationHistory)
    {
        outFile << sim.targetPos.first << " " << sim.targetPos.second << " "
                << sim.followerPos.first << " " << sim.followerPos.second << " "
                << sim.speed << " " << sim.mode << " " << sim.iterations << "\n";
    }
    outFile.close();
}

void logSimulationResult(const std::string &mode, const std::string &details, const std::string &logDetails)
{
    std::ofstream file("simulation_history.txt", std::ios::app);
    if (file.is_open())
    {
        auto now = std::chrono::system_clock::now();
        std::time_t currentTime = std::chrono::system_clock::to_time_t(now);

        std::tm localTime;
        localtime_s(&localTime, &currentTime); // Use localtime_s instead of localtime

        file << "Simulation Mode: " << mode << "\n";
        file << "Time: " << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n";
        file << logDetails << "\n"; // Log a concise version for the history
        file << "---------------------------------------------\n";
        file.close();
    }
    else
    {
        std::cerr << "Unable to open file for writing.\n";
    }
}

void loadSimulationHistory()
{
    std::ifstream inFile("simulation_history.txt");
    if (!inFile)
    {
        std::cerr << "No saved simulation history found.\n";
        return;
    }

    SimulationData simData;
    while (inFile >> simData.targetPos.first >> simData.targetPos.second >>
           simData.followerPos.first >> simData.followerPos.second >>
           simData.speed >> simData.mode >> simData.iterations)
    {
        simulationHistory.push_back(simData);
    }
    inFile.close();
}
