#include "ui/tui.h"

#include <iostream>
#include <algorithm>

#if defined(_WIN32)
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

namespace tui
{
namespace
{
enum class Key
{
    Up,
    Down,
    Enter,
    Space,
    Escape,
    Unknown
};

class TerminalMode
{
public:
    TerminalMode()
    {
#if defined(_WIN32)
        enableAnsi();
#else
        enableRawMode();
#endif
    }

    ~TerminalMode()
    {
#if !defined(_WIN32)
        disableRawMode();
#endif
    }

private:
#if !defined(_WIN32)
    termios original{};
    bool active = false;

    void enableRawMode()
    {
        if (!isatty(STDIN_FILENO))
        {
            return;
        }
        if (tcgetattr(STDIN_FILENO, &original) != 0)
        {
            return;
        }
        termios raw = original;
        raw.c_lflag &= static_cast<tcflag_t>(~(ECHO | ICANON));
        raw.c_cc[VMIN] = 1;
        raw.c_cc[VTIME] = 0;
        if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == 0)
        {
            active = true;
        }
    }

    void disableRawMode()
    {
        if (active)
        {
            tcsetattr(STDIN_FILENO, TCSAFLUSH, &original);
        }
    }
#else
    static void enableAnsi()
    {
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        if (handle == INVALID_HANDLE_VALUE)
        {
            return;
        }
        DWORD mode = 0;
        if (!GetConsoleMode(handle, &mode))
        {
            return;
        }
        mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        SetConsoleMode(handle, mode);
    }
#endif
};

Key readKey()
{
#if defined(_WIN32)
    int ch = _getch();
    if (ch == 0 || ch == 224)
    {
        int extended = _getch();
        switch (extended)
        {
        case 72:
            return Key::Up;
        case 80:
            return Key::Down;
        default:
            return Key::Unknown;
        }
    }
    if (ch == 27)
    {
        return Key::Escape;
    }
    if (ch == 13)
    {
        return Key::Enter;
    }
    if (ch == 32)
    {
        return Key::Space;
    }
    return Key::Unknown;
#else
    char ch = 0;
    if (read(STDIN_FILENO, &ch, 1) != 1)
    {
        return Key::Unknown;
    }
    if (ch == '\x1b')
    {
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(STDIN_FILENO, &readSet);
        timeval timeout{};
        timeout.tv_sec = 0;
        timeout.tv_usec = 10000;

        int ready = select(STDIN_FILENO + 1, &readSet, nullptr, nullptr, &timeout);
        if (ready <= 0)
        {
            return Key::Escape;
        }

        char seq[2];
        if (read(STDIN_FILENO, &seq[0], 1) != 1)
        {
            return Key::Escape;
        }
        if (read(STDIN_FILENO, &seq[1], 1) != 1)
        {
            return Key::Escape;
        }
        if (seq[0] == '[')
        {
            if (seq[1] == 'A')
            {
                return Key::Up;
            }
            if (seq[1] == 'B')
            {
                return Key::Down;
            }
        }
        return Key::Unknown;
    }
    if (ch == '\n' || ch == '\r')
    {
        return Key::Enter;
    }
    if (ch == ' ')
    {
        return Key::Space;
    }
    return Key::Unknown;
#endif
}

void clearScreen()
{
    std::cout << "\x1b[2J\x1b[H";
}

void renderMenu(const MenuConfig &config, const std::vector<MenuOption> &options, size_t currentIndex)
{
    clearScreen();
    std::cout << "\x1b[1m" << config.title << "\x1b[0m\n\n";
    if (!config.help.empty())
    {
        std::cout << config.help << "\n\n";
    }

    for (size_t i = 0; i < options.size(); ++i)
    {
        const auto &option = options[i];
        bool isCurrent = (i == currentIndex);
        std::string cursor = isCurrent ? "> " : "  ";
        if (isCurrent)
        {
            std::cout << "\x1b[7m";
        }

        std::string prefix = "[ ] ";
        if (option.checked)
        {
            prefix = "[x] ";
        }
        std::cout << cursor << prefix << option.label;
        if (!option.enabled)
        {
            std::cout << " (disabled)";
        }
        std::cout << "\x1b[0m\n";
    }
    std::cout << "\n";
    std::cout << std::flush;
}

} // namespace

MenuResult runMenu(const MenuConfig &config, std::vector<MenuOption> options)
{
    if (options.empty())
    {
        return MenuResult{true, -1, std::move(options)};
    }

    TerminalMode terminalMode;
    size_t currentIndex = 0;
    if (!options[currentIndex].enabled)
    {
        for (size_t i = 0; i < options.size(); ++i)
        {
            if (options[i].enabled)
            {
                currentIndex = i;
                break;
            }
        }
    }

    auto ensureSingleSelection = [&options](size_t index)
    {
        for (size_t i = 0; i < options.size(); ++i)
        {
            options[i].checked = (i == index);
        }
    };

    auto advanceSelection = [&](int direction)
    {
        if (options.size() == 1)
        {
            return;
        }
        size_t start = currentIndex;
        do
        {
            if (direction > 0)
            {
                currentIndex = (currentIndex + 1) % options.size();
            }
            else
            {
                currentIndex = (currentIndex == 0) ? options.size() - 1 : currentIndex - 1;
            }
            if (options[currentIndex].enabled)
            {
                return;
            }
        } while (currentIndex != start);
    };

    renderMenu(config, options, currentIndex);

    while (true)
    {
        Key key = readKey();
        switch (key)
        {
        case Key::Up:
            advanceSelection(-1);
            break;
        case Key::Down:
            advanceSelection(1);
            break;
        case Key::Space:
            if (!options[currentIndex].enabled)
            {
                break;
            }
            if (config.multiSelect)
            {
                options[currentIndex].checked = !options[currentIndex].checked;
            }
            else
            {
                ensureSingleSelection(currentIndex);
            }
            break;
        case Key::Enter:
        {
            if (!options[currentIndex].enabled)
            {
                break;
            }
            int selectedIndex = -1;
            if (config.multiSelect)
            {
                selectedIndex = static_cast<int>(currentIndex);
            }
            else
            {
                ensureSingleSelection(currentIndex);
                selectedIndex = static_cast<int>(currentIndex);
            }
            renderMenu(config, options, currentIndex);
            return MenuResult{false, selectedIndex, std::move(options)};
        }
        case Key::Escape:
            return MenuResult{true, -1, std::move(options)};
        default:
            break;
        }

        renderMenu(config, options, currentIndex);
    }
}

int selectSingle(const std::string &title, const std::vector<std::string> &options, const std::string &help)
{
    std::vector<MenuOption> menuOptions;
    menuOptions.reserve(options.size());
    for (const auto &option : options)
    {
        menuOptions.push_back(MenuOption{option});
    }

    MenuConfig config{title, help, false};
    MenuResult result = runMenu(config, std::move(menuOptions));
    if (result.cancelled)
    {
        return -1;
    }
    return result.selectedIndex;
}

std::vector<bool> selectMultiple(const std::string &title, const std::vector<std::string> &options, const std::string &help)
{
    std::vector<MenuOption> menuOptions;
    menuOptions.reserve(options.size());
    for (const auto &option : options)
    {
        menuOptions.push_back(MenuOption{option});
    }

    MenuConfig config{title, help, true};
    MenuResult result = runMenu(config, std::move(menuOptions));
    std::vector<bool> selections;
    selections.reserve(result.options.size());
    for (const auto &option : result.options)
    {
        selections.push_back(option.checked);
    }
    if (result.cancelled)
    {
        selections.clear();
    }
    return selections;
}
} // namespace tui

