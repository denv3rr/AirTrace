#include "ui/menu.h"
#include "ui/alerts.h"
#include "ui/menu_selection.h"
#include "ui/simulation.h"
#include "ui/scenario.h"
#include "ui/tui.h"
#include "ui/inputValidation.h"
#include "tools/audit_log.h"
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
        << " disq=" << (status.disqualifiedSources.empty() ? "none" : status.disqualifiedSources)
        << " lockout=" << (status.lockoutStatus.empty() ? "none" : status.lockoutStatus)
        << " conc=" << (status.concurrencyStatus.empty() ? "none" : status.concurrencyStatus)
        << " decision=" << (status.decisionReason.empty() ? "none" : status.decisionReason)
        << " denial=" << (status.denialReason.empty() ? "none" : status.denialReason)
        << " auth=" << status.authStatus
        << " fv_mode=" << (status.frontViewMode.empty() ? "none" : status.frontViewMode)
        << " fv_latency_ms=" << status.frontViewLatencyMs
        << " fv_drop=" << status.frontViewDroppedFrames
        << " log=" << (status.loggingStatus.empty() ? "unknown" : status.loggingStatus)
        << " sensors=" << (status.sensorStatusSummary.empty() ? "none" : status.sensorStatusSummary)
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
        "Platform Workbench",
        "Front-View Display Workbench",
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
        case ui::MainMenuAction::PlatformWorkbench:
            if (!showPlatformWorkbench())
            {
                return false;
            }
            break;
        case ui::MainMenuAction::FrontViewWorkbench:
            if (!showFrontViewWorkbench())
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

bool showFrontViewWorkbench()
{
    if (!uiEnsureAuditHealthy("front_view_workbench"))
    {
        return false;
    }
    if (!uiHasPermission("front_view_workbench") && !uiHasPermission("test_mode"))
    {
        setUiDenialReason("sim_not_authorized");
        std::cerr << "Front-view workbench not authorized. Recovery: update role permissions and retry.\n";
        return false;
    }

    const std::vector<std::string> options = {
        "Render Current Front-View Mode",
        "Cycle All Front-View Modes",
        "Export Current External I/O",
        "Back to Main Menu"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to return.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Front-View Workbench", options, buildHelp(helpBase));
        if (choice < 0 || choice == 3)
        {
            return true;
        }
        if (choice < 0 || choice >= static_cast<int>(options.size()))
        {
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }

        if (choice == 0 || choice == 1)
        {
            std::string reason;
            const bool cycleAll = (choice == 1);
            if (!uiRunFrontViewDisplaySuite(cycleAll, reason))
            {
                std::cout << "FRONT VIEW RESULT: pass=no reason=" << reason << "\n";
                std::cout << ui::buildDenialBanner(reason) << "\n";
                if (!tools::logAuditEvent("front_view_suite_run", "front-view suite failed", reason))
                {
                    setUiDenialReason("audit_unavailable");
                    std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
                    return false;
                }
                continue;
            }
            std::cout << "FRONT VIEW RESULT: pass=yes reason=" << reason << "\n";
            std::cout << "EXTERNAL IO ENVELOPE: " << uiBuildExternalIoEnvelopeJson() << "\n";
            if (!tools::logAuditEvent("front_view_suite_run", "front-view suite executed", cycleAll ? "cycle_all" : "single_mode"))
            {
                setUiDenialReason("audit_unavailable");
                std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
                return false;
            }
            continue;
        }

        std::cout << "EXTERNAL IO ENVELOPE: " << uiBuildExternalIoEnvelopeJson() << "\n";
        if (!tools::logAuditEvent("external_io_export", "exported external io envelope", "front_view"))
        {
            setUiDenialReason("audit_unavailable");
            std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
            return false;
        }
    }
}

bool showPlatformWorkbench()
{
    if (!uiEnsureAuditHealthy("platform_workbench"))
    {
        return false;
    }
    if (!uiHasPermission("platform_workbench") && !uiHasPermission("test_mode"))
    {
        setUiDenialReason("sim_not_authorized");
        std::cerr << "Platform workbench not authorized. Recovery: update role permissions and retry.\n";
        return false;
    }

    const std::vector<std::string> options = {
        "Run Selected Platform Suite",
        "Run All Platform Suites",
        "Export Current External I/O",
        "Back to Main Menu"};
    const std::string helpBase = "Use Up/Down to move, Space or Enter to select, Esc to return.";

    while (true)
    {
        int choice = tui::selectSingle("AirTrace - Platform Workbench", options, buildHelp(helpBase));
        if (choice < 0 || choice == 3)
        {
            return true;
        }
        if (choice < 0 || choice >= static_cast<int>(options.size()))
        {
            setUiDenialReason("menu_selection_invalid");
            std::cerr << "Menu selection failed. Exiting.\n";
            return false;
        }

        if (choice == 0)
        {
            const std::vector<std::string> profiles = uiListPlatformProfiles();
            int profileChoice = tui::selectSingle("AirTrace - Platform Profile", profiles, buildHelp(helpBase));
            if (profileChoice < 0)
            {
                continue;
            }
            if (profileChoice >= static_cast<int>(profiles.size()))
            {
                setUiDenialReason("menu_selection_invalid");
                std::cerr << "Profile selection failed. Returning to workbench.\n";
                continue;
            }
            const PlatformSuiteResult result = uiRunPlatformSuite(profiles[profileChoice]);
            std::cout << "PLATFORM SUITE RESULT: profile=" << result.profile
                      << " pass=" << (result.pass ? "yes" : "no")
                      << " sensors=" << (result.sensorsValidated ? "ok" : "fail")
                      << " adapter=" << (result.adapterValidated ? "ok" : "fail")
                      << " mode=" << (result.modeOutputValidated ? "ok" : "fail")
                      << " reason=" << (result.reason.empty() ? "none" : result.reason)
                      << "\n";
            std::cout << "EXTERNAL IO ENVELOPE: " << uiBuildExternalIoEnvelopeJson() << "\n";
            if (!tools::logAuditEvent("platform_suite_run", "platform suite executed", result.profile))
            {
                setUiDenialReason("audit_unavailable");
                std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
                return false;
            }
            if (!result.pass)
            {
                setUiDenialReason(result.reason);
                std::cout << ui::buildDenialBanner(result.reason) << "\n";
            }
            continue;
        }

        if (choice == 1)
        {
            const std::vector<PlatformSuiteResult> results = uiRunAllPlatformSuites();
            bool allPassed = true;
            for (const auto &entry : results)
            {
                std::cout << "PLATFORM SUITE RESULT: profile=" << entry.profile
                          << " pass=" << (entry.pass ? "yes" : "no")
                          << " sensors=" << (entry.sensorsValidated ? "ok" : "fail")
                          << " adapter=" << (entry.adapterValidated ? "ok" : "fail")
                          << " mode=" << (entry.modeOutputValidated ? "ok" : "fail")
                          << " reason=" << (entry.reason.empty() ? "none" : entry.reason)
                          << "\n";
                if (!entry.pass)
                {
                    allPassed = false;
                }
            }
            std::cout << "EXTERNAL IO ENVELOPE: " << uiBuildExternalIoEnvelopeJson() << "\n";
            if (!tools::logAuditEvent("platform_suite_cycle", "all platform suites executed", allPassed ? "pass" : "fail"))
            {
                setUiDenialReason("audit_unavailable");
                std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
                return false;
            }
            if (!allPassed)
            {
                setUiDenialReason("platform_suite_failed");
                std::cout << ui::buildDenialBanner("platform_suite_failed") << "\n";
            }
            continue;
        }

        std::cout << "EXTERNAL IO ENVELOPE: " << uiBuildExternalIoEnvelopeJson() << "\n";
        if (!tools::logAuditEvent("external_io_export", "exported external io envelope", getUiStatus().platformProfile))
        {
            setUiDenialReason("audit_unavailable");
            std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
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

