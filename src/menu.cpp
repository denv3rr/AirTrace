#include "menu.h"
#include "inputValidation.h"
#include "simulation.h"
#include "scenario.h"
#include <iostream>

void showMainMenu()
{
    int choice = 0;

    while (choice != 5) // Adding a new option for deleting simulations
    {
        std::cout << "\n\033[32mAirTrace\033[0m\n\n";
        std::cout << "Main Menu\n\n";
        std::cout << "1. Real Input Mode\n";
        std::cout << "2. Test Mode (Manual Configuration)\n";
        std::cout << "3. View and Rerun Previous Simulations\n";
        std::cout << "4. Delete a Previous Simulation\n"; // New option
        std::cout << "5. Exit\n\n";

        choice = getValidatedIntInput("Select an option: ", 1, 5);

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
            deletePreviousSimulation(); // Call to delete a simulation
            break;
        case 5:
            std::cout << "Exiting the system.\n";
            break;
        default:
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
        }
    }
}
