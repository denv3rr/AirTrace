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
    assert(denialRecoveryHint("platform_profile_invalid") == "select a supported platform profile and retry");
    assert(denialRecoveryHint("platform_suite_failed") == "review platform suite diagnostics and adapter status");
    assert(denialRecoveryHint("front_view_disabled") == "set front_view.enabled=true and retry");
    assert(denialRecoveryHint("front_view_sensor_unavailable") == "enable front_view.spoof.enabled or connect an approved sensor feed");
    assert(denialRecoveryHint("front_view_mode_invalid") == "verify front_view.display_families and auto_cycle.order values");
    assert(denialRecoveryHint("front_view_spoof_invalid") == "verify front_view.spoof.pattern and front_view.spoof.motion_profile");
    assert(denialRecoveryHint("render_latency_exceeded") == "reduce front-view pipeline load or increase latency budget");
    assert(denialRecoveryHint("unknown_reason") == "review logs and configuration");

    assert(buildDenialBanner("") == "");
    assert(buildDenialBanner("input_unavailable").find("WARNING: input_unavailable") == 0);
    assert(buildDenialBanner("input_unavailable").find("Recovery: verify input stream") != std::string::npos);
}
