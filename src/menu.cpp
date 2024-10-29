#include "menu.h"
#include "inputValidation.h"
#include "simulation.h"
#include "scenario.h"
#include <iostream>
#include <fstream>

void showMainMenu()
{
    int choice = 0;

    while (choice != 5)
    {
        std::cout << "\n\033[32mAirTrace\033[0m\n\n";
        std::cout << "Main Menu\n\n";
        std::cout << "1. Scenario Mode\n";
        std::cout << "2. Test Mode\n";
        std::cout << "3. View and Rerun Previous Simulations\n";
        std::cout << "4. Delete a Previous Simulation\n";
        std::cout << "5. Exit\n\n";

        choice = getValidatedIntInput("Select an option: ", 1, 5);

        switch (choice)
        {
        case 1:
        {
            Object follower(2, "Follower", {0, 0});
            int speed = 100;
            int iterations = 100;
            runScenarioMainMode(follower, speed, iterations); // Ensure this function is defined and accessible
            break;
        }
        case 2:
            showTestMenu();
            break;
        case 3:
            viewAndRerunPreviousSimulations();
            break;
        case 4:
            deletePreviousSimulation();
            break;
        case 5:
            std::cout << "Exiting the system.\n";
            break;
        default:
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
        }
    }
}

void showTestMenu()
{
    int choice = 0;

    while (choice != 3)
    {
        std::cout << "\nTest Menu\n";
        std::cout << "1. Run Test Scenario Mode\n";
        std::cout << "2. View Test Logs\n";
        std::cout << "3. Back to Main Menu\n";

        choice = getValidatedIntInput("Select an option: ", 1, 3);

        switch (choice)
        {
        case 1:
            runTestScenarioMode();
            break;
        case 2:
            viewTestLogs();
            break;
        case 3:
            std::cout << "Returning to Main Menu.\n";
            break;
        default:
            std::cout << "\033[31mInvalid choice. Please try again.\033[0m\n";
        }
    }
}

void viewTestLogs()
{
    std::ifstream logFile("test_logs.txt");
    if (!logFile)
    {
        std::cerr << "No test logs available.\n";
        return;
    }

    std::string line;
    while (std::getline(logFile, line))
    {
        std::cout << line << "\n";
    }
    logFile.close();
}