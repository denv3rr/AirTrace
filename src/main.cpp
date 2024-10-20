#include "../include/main.h"
#include <iostream>
#include <limits>

// Function declarations for the menu
void showMainMenu();
void runRealInputScenario();
void runTestMode();

int main()
{
    showMainMenu();
    return 0;
}

void showMainMenu()
{
    int choice = 0;

    while (choice != 3)
    {
        std::cout << "\n\033[32mAirTrace Tracking System\033[0m\n";
        std::cout << "1. Real Input Scenario Mode\n";
        std::cout << "2. Test Mode (Manual Configuration)\n";
        std::cout << "3. Exit\n";
        std::cout << "Select an option: ";
        std::cin >> choice;

        switch (choice)
        {
        case 1:
            runRealInputScenario();
            break;
        case 2:
            runTestMode();
            break;
        case 3:
            std::cout << "Exiting the system.\n";
            break;
        default:
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

// The existing real input scenario
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

    std::cout << "\nTracking system stopped.\n";
}

// Test Mode to manually set parameters for simulation
void runTestMode()
{
    std::cout << "\n\033[32mTest Mode\033[0m\n";

    // Get user input for positions and speed
    int targetX, targetY, followerX, followerY, speed;
    std::string trackingMode;

    std::cout << "Enter initial target position (X Y): ";
    std::cin >> targetX >> targetY;

    std::cout << "Enter initial follower position (X Y): ";
    std::cin >> followerX >> followerY;

    std::cout << "Enter movement speed (1-10): ";
    std::cin >> speed;

    std::cout << "Choose tracking mode (prediction/kalman/heat_signature/dead_reckoning): ";
    std::cin >> trackingMode;

    // Create objects with user-defined positions
    Object target(1, "Target", {targetX, targetY});
    Object follower(2, "Follower", {followerX, followerY});

    // Initialize tracker with manual config
    Tracker tracker(follower);
    tracker.setTrackingMode(trackingMode);
    tracker.setTarget(target);

    // Run simulation in test mode with user-defined speed
    for (int i = 0; i < speed; i++)
    {
        tracker.update();
        // Simulate delay or time step
    }

    std::cout << "\nTest simulation finished.\n";
}
