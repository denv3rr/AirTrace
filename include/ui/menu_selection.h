#ifndef MENU_SELECTION_H
#define MENU_SELECTION_H

#include <cstddef>

namespace ui
{
enum class MainMenuAction
{
    Scenario,
    TestMenu,
    ViewHistory,
    DeleteHistory,
    Exit,
    Cancel,
    InputError
};

enum class TestMenuAction
{
    RunScenarioTest,
    ViewLogs,
    Back,
    Cancel,
    InputError
};

MainMenuAction resolveMainMenuAction(int selectedIndex, size_t optionCount);
TestMenuAction resolveTestMenuAction(int selectedIndex, size_t optionCount);
} // namespace ui

#endif // MENU_SELECTION_H
