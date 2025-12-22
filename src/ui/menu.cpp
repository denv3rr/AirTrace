#include "ui/menu.h"
#include "ui/simulation.h"
#include "ui/scenario.h"
#include "ui/tui.h"
#include <iostream>
#include <fstream>

void showMainMenu()
{
    const std::vector<std::string> options = {
        "Scenario Mode",
        "Test Mode",
        "View and Rerun Previous Simulations",
        "Delete a Previous Simulation",
        "Exit"};
    const std::string help = "Use Up/Down to move, Space or Enter to select, Esc to exit.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Main Menu", options, help);
        if (choice < 0 || choice == 4)
        {
            std::cout << "Exiting the system.\n";
            return;
        }

        switch (choice + 1)
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
    const std::vector<std::string> options = {
        "Run Test Scenario Mode",
        "View Test Logs",
        "Back to Main Menu"};
    const std::string help = "Use Up/Down to move, Space or Enter to select, Esc to return.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Test Menu", options, help);
        if (choice < 0 || choice == 2)
        {
            std::cout << "Returning to Main Menu.\n";
            return;
        }

        switch (choice + 1)
        {
        case 1:
            runTestScenarioMode();
            break;
        case 2:
            viewTestLogs();
            break;
        case 3:
            std::cout << "Returning to Main Menu.\n";
            return;
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

