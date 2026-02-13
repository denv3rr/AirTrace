#include "ui/menu_selection.h"

namespace
{
constexpr size_t kMainMenuOptionCount = 7;
constexpr size_t kTestMenuOptionCount = 3;
} // namespace

namespace ui
{
MainMenuAction resolveMainMenuAction(int selectedIndex, size_t optionCount)
{
    if (optionCount != kMainMenuOptionCount)
    {
        return MainMenuAction::InputError;
    }
    if (selectedIndex < 0)
    {
        return MainMenuAction::Cancel;
    }
    if (static_cast<size_t>(selectedIndex) >= optionCount)
    {
        return MainMenuAction::InputError;
    }

    switch (selectedIndex)
    {
    case 0:
        return MainMenuAction::Scenario;
    case 1:
        return MainMenuAction::TestMenu;
    case 2:
        return MainMenuAction::PlatformWorkbench;
    case 3:
        return MainMenuAction::FrontViewWorkbench;
    case 4:
        return MainMenuAction::ViewHistory;
    case 5:
        return MainMenuAction::DeleteHistory;
    case 6:
        return MainMenuAction::Exit;
    default:
        return MainMenuAction::InputError;
    }
}

TestMenuAction resolveTestMenuAction(int selectedIndex, size_t optionCount)
{
    if (optionCount != kTestMenuOptionCount)
    {
        return TestMenuAction::InputError;
    }
    if (selectedIndex < 0)
    {
        return TestMenuAction::Cancel;
    }
    if (static_cast<size_t>(selectedIndex) >= optionCount)
    {
        return TestMenuAction::InputError;
    }

    switch (selectedIndex)
    {
    case 0:
        return TestMenuAction::RunScenarioTest;
    case 1:
        return TestMenuAction::ViewLogs;
    case 2:
        return TestMenuAction::Back;
    default:
        return TestMenuAction::InputError;
    }
}
} // namespace ui
