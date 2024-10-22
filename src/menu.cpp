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
        std::cout << "4. Exit\n\n";
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