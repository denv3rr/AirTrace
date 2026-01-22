#include "ui/alerts.h"

namespace ui
{
std::string denialRecoveryHint(const std::string &denialReason)
{
    if (denialReason == "input_unavailable")
    {
        return "verify input stream and restart";
    }
    if (denialReason == "menu_input_unavailable")
    {
        return "use an interactive terminal and retry";
    }
    if (denialReason == "menu_selection_invalid")
    {
        return "restart the menu and select a valid option";
    }
    if (denialReason == "selection_cancelled")
    {
        return "reopen the menu to select a mode";
    }
    if (denialReason == "config_invalid")
    {
        return "verify configs/sim_default.cfg and rerun";
    }
    if (denialReason == "test_logs_missing")
    {
        return "run Test Mode to generate logs";
    }
    if (denialReason == "history_save_failed")
    {
        return "verify write permissions for simulation_history.txt";
    }
    if (denialReason == "test_log_write_failed")
    {
        return "verify write permissions for test_logs.txt";
    }
    return "review logs and configuration";
}

std::string buildDenialBanner(const std::string &denialReason)
{
    if (denialReason.empty())
    {
        return "";
    }
    std::string banner = "WARNING: " + denialReason + "\n";
    banner += "Recovery: " + denialRecoveryHint(denialReason);
    return banner;
}
} // namespace ui
