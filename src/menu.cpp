#include "menu.h"
#include "inputValidation.h"
#include "simulation.h"
#include "scenario.h"
#include <iostream>

void showMainMenu()
{
    int choice = 0;

    while (choice != 4)
    {
        std::cout << "\n\033[32mAirTrace Tracking System\033[0m\n\n";
        std::cout << "Main Menu: Select Mode\n\n";
        std::cout << "1. Real Input Mode\n";
        std::cout << "2. Test Mode (Manual Configuration)\n";
        std::cout << "3. View and Rerun Previous Simulations\n";
        std::cout << "4. Exit\n";
        std::cout << "Select an option: ";

        choice = getValidatedIntInput("Select an option: ", 1, 4);

        switch (choice)
        {
        case 1:
            runRealInputScenario();
            break;
        case 2:
            runTestMode();
            break;
        case 3:
            viewAndRerunPreviousSimulations();
            break;
        case 4:
            std::cout << "Exiting the system.\n";
            break;
        default:
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
        }
    }
}

void runTestMode()
{
    std::cout << "\n\033[32mTest Mode\033[0m\n";

    int speed, iterations, modeChoice;
    std::string trackingMode;

    speed = getValidatedIntInput("Enter movement speed (1-10): ", 1, 10);
    iterations = getValidatedIntInput("Enter number of iterations for the simulation (0 for infinite): ", 0, 1000);

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
        std::cout << "\033[31mInvalid choice. Defaulting to prediction mode.\033[0m\n";
        trackingMode = "prediction";
    }

    if (trackingMode == "heat_signature")
    {
        simulateHeatSeeking(speed, iterations);
    }
    else
    {
        // Construct SimulationData object and run
        SimulationData simData = {{0, 0}, {0, 0}, speed, trackingMode, iterations};
        simulateManualConfig(simData);
    }
}
