#include "ui/menu_selection.h"

#include <cassert>

void runMenuSelectionTests()
{
    using ui::MainMenuAction;
    using ui::TestMenuAction;

    assert(ui::resolveMainMenuAction(-1, 7) == MainMenuAction::Cancel);
    assert(ui::resolveMainMenuAction(0, 7) == MainMenuAction::Scenario);
    assert(ui::resolveMainMenuAction(1, 7) == MainMenuAction::TestMenu);
    assert(ui::resolveMainMenuAction(2, 7) == MainMenuAction::PlatformWorkbench);
    assert(ui::resolveMainMenuAction(3, 7) == MainMenuAction::FrontViewWorkbench);
    assert(ui::resolveMainMenuAction(4, 7) == MainMenuAction::ViewHistory);
    assert(ui::resolveMainMenuAction(5, 7) == MainMenuAction::DeleteHistory);
    assert(ui::resolveMainMenuAction(6, 7) == MainMenuAction::Exit);
    assert(ui::resolveMainMenuAction(7, 7) == MainMenuAction::InputError);
    assert(ui::resolveMainMenuAction(0, 6) == MainMenuAction::InputError);

    assert(ui::resolveTestMenuAction(-1, 3) == TestMenuAction::Cancel);
    assert(ui::resolveTestMenuAction(0, 3) == TestMenuAction::RunScenarioTest);
    assert(ui::resolveTestMenuAction(1, 3) == TestMenuAction::ViewLogs);
    assert(ui::resolveTestMenuAction(2, 3) == TestMenuAction::Back);
    assert(ui::resolveTestMenuAction(3, 3) == TestMenuAction::InputError);
    assert(ui::resolveTestMenuAction(0, 2) == TestMenuAction::InputError);

}
