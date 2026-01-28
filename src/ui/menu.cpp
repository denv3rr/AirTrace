#include "ui/menu.h"
#include "ui/alerts.h"
#include "ui/menu_selection.h"
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
        << " parent=" << (status.parentProfile.empty() ? "none" : status.parentProfile)
        << " modules=" << (status.childModules.empty() ? "none" : status.childModules)
        << " source=" << status.activeSource
        << " contributors=" << (status.contributors.empty() ? "none" : status.contributors)
        << " conf=" << status.modeConfidence
        << " conc=" << (status.concurrencyStatus.empty() ? "none" : status.concurrencyStatus)
        << " decision=" << (status.decisionReason.empty() ? "none" : status.decisionReason)
        << " denial=" << (status.denialReason.empty() ? "none" : status.denialReason)
        << " auth=" << status.authStatus
        << " log=" << (status.loggingStatus.empty() ? "unknown" : status.loggingStatus)
        << " seed=" << status.seed
        << " det=" << (status.deterministic ? "on" : "off");
    if (!status.denialReason.empty())
    {
        out << "\n" << ui::buildDenialBanner(status.denialReason);
    }
    return out.str();
}
} // namespace

bool showMainMenu()
{
    const std::vector<std::string> options = {
        "Scenario Mode",
        "Test Mode",
        "View and Rerun Previous Simulations",
        "Delete a Previous Simulation",
        "Exit"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to exit.";

    if (!tui::isInteractiveInput())
    {
        setUiDenialReason("menu_input_unavailable");
        std::cerr << "Menu input unavailable. Exiting.\n";
        return false;
    }

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Main Menu", options, buildHelp(helpBase));
        ui::MainMenuAction action = ui::resolveMainMenuAction(choice, options.size());
        if (action == ui::MainMenuAction::Cancel || action == ui::MainMenuAction::Exit)
        {
            std::cout << "Exiting the system.\n";
            return true;
        }
        if (action == ui::MainMenuAction::InputError)
        {
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }

        switch (action)
        {
        case ui::MainMenuAction::Scenario:
        {
            if (!uiEnsureAuditHealthy("scenario_mode"))
            {
                return false;
            }
            int gpsTimeoutSeconds = 10;
            int heatTimeoutSeconds = 10;
            if (!tryGetValidatedIntInput("Enter GPS timeout seconds (1-120): ", 1, 120, gpsTimeoutSeconds) ||
                !tryGetValidatedIntInput("Enter heat timeout seconds (1-120): ", 1, 120, heatTimeoutSeconds))
            {
                setUiDenialReason("input_unavailable");
                std::cerr << "Input unavailable. Exiting.\n";
                return false;
            }
            Object follower(2, "Follower", {0, 0});
            if (!runScenarioMainMode(follower, gpsTimeoutSeconds, heatTimeoutSeconds))
            {
                return false;
            }
            break;
        }
        case ui::MainMenuAction::TestMenu:
            if (!uiEnsureAuditHealthy("test_menu"))
            {
                return false;
            }
            if (!uiHasPermission("test_mode"))
            {
                setUiDenialReason("sim_not_authorized");
                std::cerr << "Test mode not authorized. Recovery: update role permissions and retry.\n";
                return false;
            }
            if (!showTestMenu())
            {
                return false;
            }
            break;
        case ui::MainMenuAction::ViewHistory:
            if (!uiEnsureAuditHealthy("view_history"))
            {
                return false;
            }
            if (!uiHasPermission("simulation_history"))
            {
                setUiDenialReason("sim_not_authorized");
                std::cerr << "History access not authorized. Recovery: update role permissions and retry.\n";
                return false;
            }
            viewAndRerunPreviousSimulations();
            break;
        case ui::MainMenuAction::DeleteHistory:
            if (!uiEnsureAuditHealthy("delete_history"))
            {
                return false;
            }
            if (!uiHasPermission("simulation_delete"))
            {
                setUiDenialReason("sim_not_authorized");
                std::cerr << "Delete not authorized. Recovery: update role permissions and retry.\n";
                return false;
            }
            deletePreviousSimulation();
            break;
        default:
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }
    }
}

bool showTestMenu()
{
    const std::vector<std::string> options = {
        "Run Test Scenario Mode",
        "View Test Logs",
        "Back to Main Menu"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to return.";

    if (!tui::isInteractiveInput())
    {
        setUiDenialReason("menu_input_unavailable");
        std::cerr << "Menu input unavailable. Exiting.\n";
        return false;
    }

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Test Menu", options, buildHelp(helpBase));
        ui::TestMenuAction action = ui::resolveTestMenuAction(choice, options.size());
        if (action == ui::TestMenuAction::Cancel || action == ui::TestMenuAction::Back)
        {
            std::cout << "Returning to Main Menu.\n";
            return true;
        }
        if (action == ui::TestMenuAction::InputError)
        {
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }

        switch (action)
        {
        case ui::TestMenuAction::RunScenarioTest:
            if (!runTestScenarioMode())
            {
                return false;
            }
            break;
        case ui::TestMenuAction::ViewLogs:
            if (!uiHasPermission("test_mode"))
            {
                setUiDenialReason("sim_not_authorized");
                std::cerr << "Test logs not authorized. Recovery: update role permissions and retry.\n";
                return false;
            }
            viewTestLogs();
            break;
        default:
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }
    }
}

void viewTestLogs()
{
    std::ifstream logFile("test_logs.txt");
    if (!logFile)
    {
        setUiDenialReason("test_logs_missing");
        std::cerr << "No test logs available. Recovery: run Test Mode to generate logs.\n";
        return;
    }

    std::string line;
    while (std::getline(logFile, line))
    {
        std::cout << line << "\n";
    }
    logFile.close();
}

