#ifndef TUI_H
#define TUI_H

#include <string>
#include <vector>

namespace tui
{
struct MenuOption
{
    std::string label;
    bool checked = false;
    bool enabled = true;
};

struct MenuConfig
{
    std::string title;
    std::string help;
    bool multiSelect = false;
};

struct MenuResult
{
    bool cancelled = false;
    int selectedIndex = -1;
    std::vector<MenuOption> options;
};

MenuResult runMenu(const MenuConfig &config, std::vector<MenuOption> options);
int selectSingle(const std::string &title, const std::vector<std::string> &options, const std::string &help);
std::vector<bool> selectMultiple(const std::string &title, const std::vector<std::string> &options, const std::string &help);
} // namespace tui

#endif // TUI_H

