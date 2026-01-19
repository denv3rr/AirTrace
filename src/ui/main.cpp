#include "ui/main.h"

int main()
{
    if (!initializeUiContext("configs/sim_default.cfg"))
    {
        std::cerr << "Warning: unable to load configs/sim_default.cfg. UI status uses defaults.\n";
    }
    loadSimulationHistory();
    showMainMenu();
    saveSimulationHistory();
    return 0;
}
