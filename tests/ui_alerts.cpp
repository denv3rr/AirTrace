#include "ui/alerts.h"

#include <cassert>

void runAlertsTests()
{
    using ui::buildDenialBanner;
    using ui::denialRecoveryHint;

    assert(denialRecoveryHint("input_unavailable") == "verify input stream and restart");
    assert(denialRecoveryHint("menu_input_unavailable") == "use an interactive terminal and retry");
    assert(denialRecoveryHint("menu_selection_invalid") == "restart the menu and select a valid option");
    assert(denialRecoveryHint("selection_cancelled") == "reopen the menu to select a mode");
    assert(denialRecoveryHint("config_invalid") == "verify configs/sim_default.cfg and rerun");
    assert(denialRecoveryHint("test_logs_missing") == "run Test Mode to generate logs");
    assert(denialRecoveryHint("history_save_failed") == "verify write permissions for simulation_history.txt");
    assert(denialRecoveryHint("test_log_write_failed") == "verify write permissions for test_logs.txt");
    assert(denialRecoveryHint("unknown_reason") == "review logs and configuration");

    assert(buildDenialBanner("") == "");
    assert(buildDenialBanner("input_unavailable").find("WARNING: input_unavailable") == 0);
    assert(buildDenialBanner("input_unavailable").find("Recovery: verify input stream") != std::string::npos);
}
