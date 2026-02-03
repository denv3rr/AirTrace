#include "ui/scenario.h"

#include "core/Tracker.h"
#include "core/GPSAlgorithm.h"
#include "ui/inputValidation.h"
#include "ui/simulation.h"
#include "tools/audit_log.h"

#include <atomic>
#include <iostream>
#include <vector>
#include <cmath>
#include <thread>
#include <chrono>

namespace
{
bool auditOrDeny(const std::string &eventType, const std::string &message, const std::string &detail, const std::string &context)
{
    if (tools::logAuditEvent(eventType, message, detail))
    {
        return true;
    }
    setUiDenialReason("audit_unavailable");
    std::cerr << "Audit logging unavailable. Recovery: verify audit log sink and retry.\n";
    std::cerr << "Context: " << context << "\n";
    return false;
}
}

bool runScenarioMode(Object &follower, int gpsTimeout, int heatTimeout, std::atomic<bool> *exitRequested)
{
    Tracker tracker(follower);
    tracker.setTrackingMode("gps");
    setUiActiveSource("gps");
    setUiDecisionReason("scenario_gps");
    uiRenderStatusBanner("scenario_gps");
    if (!auditOrDeny("scenario_start", "scenario mode started", "gps", "scenario_start"))
    {
        return false;
    }

    int gpsAttempts = 0;
    while (gpsAttempts++ < gpsTimeout && tracker.isTrackingActive())
    {
        if (exitRequested && exitRequested->load())
        {
            setUiDenialReason("operator_abort");
            tools::logAuditEvent("operator_abort", "scenario aborted", "gps");
            return false;
        }
        if (inputStreamAvailable() && std::cin.rdbuf()->in_avail() > 0)
        {
            std::string input;
            if (std::getline(std::cin, input) && (input == "x" || input == "X"))
            {
                setUiDenialReason("operator_abort");
                tools::logAuditEvent("operator_abort", "scenario aborted", "gps");
                return false;
            }
        }
        tracker.update();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    if (tracker.isTrackingActive())
    {
        tracker.setTrackingMode("heat_signature");
        setUiActiveSource("heat_signature");
        setUiDecisionReason("scenario_heat");
        uiRenderStatusBanner("scenario_heat");
        if (!auditOrDeny("scenario_mode_switch", "scenario mode switched", "heat_signature", "scenario_switch"))
        {
            return false;
        }
        int heatAttempts = 0;
        while (heatAttempts++ < heatTimeout && tracker.isTrackingActive())
        {
            if (exitRequested && exitRequested->load())
            {
                setUiDenialReason("operator_abort");
                tools::logAuditEvent("operator_abort", "scenario aborted", "heat_signature");
                return false;
            }
            if (inputStreamAvailable() && std::cin.rdbuf()->in_avail() > 0)
            {
                std::string input;
                if (std::getline(std::cin, input) && (input == "x" || input == "X"))
                {
                    setUiDenialReason("operator_abort");
                    tools::logAuditEvent("operator_abort", "scenario aborted", "heat_signature");
                    return false;
                }
            }
            tracker.update();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    if (!auditOrDeny("scenario_complete", "scenario mode completed", "", "scenario_complete"))
    {
        return false;
    }
    return true;
}

bool runScenarioMainMode(Object &follower, int gpsTimeout, int heatTimeout)
{
    if (!uiEnsureAuditHealthy("scenario_mode"))
    {
        return false;
    }
    if (inputStreamAvailable())
    {
        return runScenarioMode(follower, gpsTimeout, heatTimeout, nullptr);
    }
    setUiActiveSource("hold");
    setUiDenialReason("input_unavailable");
    std::cerr << "Input stream unavailable. Exiting.\n";
    return false;
}


