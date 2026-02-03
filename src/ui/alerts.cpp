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
    if (denialReason == "invalid_ladder" || denialReason == "invalid_mode")
    {
        return "verify mode.ladder_order configuration and restart";
    }
    if (denialReason == "no_measurement")
    {
        return "verify sensor data stream and retry";
    }
    if (denialReason == "stale")
    {
        return "verify sensor freshness thresholds or data feed timing";
    }
    if (denialReason == "low_confidence")
    {
        return "verify sensor calibration and confidence thresholds";
    }
    if (denialReason == "lockout")
    {
        return "allow lockout to expire or reset per policy";
    }
    if (denialReason == "not_permitted")
    {
        return "verify permitted sensors and policy authorization";
    }
    if (denialReason == "celestial_disallowed")
    {
        return "enable celestial use in policy or select alternate source";
    }
    if (denialReason == "dataset_unavailable")
    {
        return "provide valid celestial dataset paths and hashes";
    }
    if (denialReason == "auth_unavailable")
    {
        return "provide a verified authorization bundle or change run mode per policy";
    }
    if (denialReason == "auth_denied")
    {
        return "update authorization bundle to allow the requested mode";
    }
    if (denialReason == "provenance_denied")
    {
        return "align inputs with allowed provenance or update policy";
    }
    if (denialReason == "provenance_unknown")
    {
        return "ensure inputs are tagged with provenance or adjust unknown_action policy";
    }
    if (denialReason == "provenance_mixed")
    {
        return "provide single-provenance inputs or enable allow_mixed policy";
    }
    if (denialReason == "provenance_unconfigured")
    {
        return "configure allowed provenance inputs and retry";
    }
    if (denialReason == "residual_conflict")
    {
        return "verify sensor alignment and calibration; degrade to safe mode";
    }
    if (denialReason == "residual_unaligned")
    {
        return "synchronize sensor timestamps and retry";
    }
    if (denialReason == "missing")
    {
        return "verify sensor availability and configuration";
    }
    if (denialReason == "unhealthy_count")
    {
        return "verify sensor health and minimum healthy count thresholds";
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
