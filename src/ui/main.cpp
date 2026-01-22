#include "ui/main.h"

int main()
{
    if (!initializeUiContext("configs/sim_default.cfg"))
    {
        std::cerr << "Error: unable to load configs/sim_default.cfg. Exiting.\n";
        return 1;
    }
    loadSimulationHistory();
    if (!showMainMenu())
    {
        saveSimulationHistory();
        return 1;
    }
    saveSimulationHistory();
    return 0;
}
