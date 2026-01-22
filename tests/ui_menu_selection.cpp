#include "ui/menu_selection.h"

#include <cassert>

void runMenuSelectionTests()
{
    using ui::MainMenuAction;
    using ui::TestMenuAction;

    assert(ui::resolveMainMenuAction(-1, 5) == MainMenuAction::Cancel);
    assert(ui::resolveMainMenuAction(0, 5) == MainMenuAction::Scenario);
    assert(ui::resolveMainMenuAction(1, 5) == MainMenuAction::TestMenu);
    assert(ui::resolveMainMenuAction(2, 5) == MainMenuAction::ViewHistory);
    assert(ui::resolveMainMenuAction(3, 5) == MainMenuAction::DeleteHistory);
    assert(ui::resolveMainMenuAction(4, 5) == MainMenuAction::Exit);
    assert(ui::resolveMainMenuAction(5, 5) == MainMenuAction::InputError);
    assert(ui::resolveMainMenuAction(0, 4) == MainMenuAction::InputError);

    assert(ui::resolveTestMenuAction(-1, 3) == TestMenuAction::Cancel);
    assert(ui::resolveTestMenuAction(0, 3) == TestMenuAction::RunScenarioTest);
    assert(ui::resolveTestMenuAction(1, 3) == TestMenuAction::ViewLogs);
    assert(ui::resolveTestMenuAction(2, 3) == TestMenuAction::Back);
    assert(ui::resolveTestMenuAction(3, 3) == TestMenuAction::InputError);
    assert(ui::resolveTestMenuAction(0, 2) == TestMenuAction::InputError);

}
