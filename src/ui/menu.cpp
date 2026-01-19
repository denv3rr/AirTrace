#include "ui/menu.h"
#include "ui/simulation.h"
#include "ui/scenario.h"
#include "ui/tui.h"
#include "ui/inputValidation.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace
{
std::string buildHelp(const std::string &baseHelp)
{
    const UiStatus &status = getUiStatus();
    std::ostringstream out;
    out << baseHelp << "\n"
        << "Status: profile=" << status.platformProfile
        << " source=" << status.activeSource
        << " auth=" << status.authStatus
        << " seed=" << status.seed
        << " det=" << (status.deterministic ? "on" : "off");
    return out.str();
}
} // namespace

void showMainMenu()
{
    const std::vector<std::string> options = {
        "Scenario Mode",
        "Test Mode",
        "View and Rerun Previous Simulations",
        "Delete a Previous Simulation",
        "Exit"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to exit.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Main Menu", options, buildHelp(helpBase));
        if (choice < 0 || choice == 4)
        {
            std::cout << "Exiting the system.\n";
            return;
        }

        switch (choice + 1)
        {
        case 1:
        {
            int gpsTimeoutSeconds = 10;
            int heatTimeoutSeconds = 10;
            if (!tryGetValidatedIntInput("Enter GPS timeout seconds (1-120): ", 1, 120, gpsTimeoutSeconds) ||
                !tryGetValidatedIntInput("Enter heat timeout seconds (1-120): ", 1, 120, heatTimeoutSeconds))
            {
                std::cout << "Input unavailable. Returning to menu.\n";
                break;
            }
            Object follower(2, "Follower", {0, 0});
            runScenarioMainMode(follower, gpsTimeoutSeconds, heatTimeoutSeconds);
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
            std::cout << "Invalid choice. Please try again.\n";
        }
    }
}

void showTestMenu()
{
    const std::vector<std::string> options = {
        "Run Test Scenario Mode",
        "View Test Logs",
        "Back to Main Menu"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to return.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Test Menu", options, buildHelp(helpBase));
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
            std::cout << "Invalid choice. Please try again.\n";
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

