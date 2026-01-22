#include "ui/input_harness.h"
#include "ui/menu.h"
#include "ui/simulation.h"

#include <cstdlib>
#include <iostream>

int main()
{
    const char *enabled = std::getenv("AIRTRACE_TEST_HARNESS");
    if (enabled == nullptr || std::string(enabled) != "1")
    {
        std::cerr << "Harness disabled. Set AIRTRACE_TEST_HARNESS=1 to run.\n";
        return 1;
    }

    const char *commandPath = std::getenv("AIRTRACE_HARNESS_COMMANDS");
    std::string path = commandPath ? commandPath : "configs/harness_commands.txt";

    ui::InputHarness harness(path);
    if (!harness.isEnabled())
    {
        std::cerr << "Harness input unavailable. Verify command file path.\n";
        return 1;
    }

    ui::setInputHarness(&harness);

    if (!initializeUiContext("configs/sim_default.cfg"))
    {
        std::cerr << "Warning: unable to load configs/sim_default.cfg. UI status uses defaults.\n";
    }
    loadSimulationHistory();
    bool ok = showMainMenu();
    saveSimulationHistory();
    return ok ? 0 : 1;
}
