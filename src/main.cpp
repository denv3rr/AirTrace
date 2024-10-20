#include "../include/main.h"

// Function declarations for the menu
void showMainMenu();
void runRealInputScenario();
void runTestMode();
int getValidatedIntInput(const std::string &prompt, int min, int max);
void clearInputStream();

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
        std::cout << "\n\033[32mAirTrace Tracking System\033[0m\n\n";
        std::cout << "Main Menu: Select Mode\n\n";
        std::cout << "1. Real Input Mode\n";
        std::cout << "2. Test Mode (Manual Configuration)\n";
        std::cout << "3. Exit\n";
        std::cout << "Select an option: ";

        choice = getValidatedIntInput("Select an option: ", 1, 3); // Menu validation

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
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
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

    std::cout << "\nTracking system stopped.\n\n";
    std::cout << "*********************************************\n\n";
}

// Test Mode to manually set parameters for simulation
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
    iterations = getValidatedIntInput("Enter number of iterations for the simulation: ", 1, 100);

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

    // Run simulation in test mode with user-defined iterations and speed
    for (int i = 0; i < iterations; i++)
    {
        tracker.update();
        // Simulate delay or time step
        std::this_thread::sleep_for(std::chrono::milliseconds(500 / speed)); // Slows down based on speed
    }

    std::cout << "\n\n\nTest simulation finished.\n\n";
    std::cout << "*********************************************\n\n";
}

// Function to validate integer input within a range
int getValidatedIntInput(const std::string &prompt, int min, int max)
{
    int input;

    while (true)
    {
        std::cout << prompt;
        std::cin >> input;

        // Check for invalid input or out of range
        if (std::cin.fail() || input < min || input > max)
        {
            std::cout << "\033[31mInvalid input. Please enter a number between " << min << " and " << max << ".\033[0m\n\n\n";
            clearInputStream(); // Clear the invalid input
        }
        else
        {
            return input; // Valid input
        }
    }
}

// Function to clear the input stream in case of invalid input
void clearInputStream()
{
    std::cin.clear();                                                   // Clear the error flag
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Discard invalid input
}
