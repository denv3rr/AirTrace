#include "ui/input_harness.h"

#if defined(AIRTRACE_TEST_HARNESS)
#include <cstdlib>
#include <fstream>
#include <sstream>

namespace ui
{
namespace
{
InputHarness *activeHarness = nullptr;
}

InputHarness::InputHarness(std::string path)
    : commandPath(std::move(path))
{
    const char *enabledEnv = std::getenv("AIRTRACE_TEST_HARNESS");
    enabled = enabledEnv != nullptr && std::string(enabledEnv) == "1" && !commandPath.empty();
}

bool InputHarness::isEnabled() const
{
    return enabled;
}

bool InputHarness::loadNewCommands()
{
    if (!enabled)
    {
        return false;
    }
    std::ifstream file(commandPath);
    if (!file)
    {
        return false;
    }
    file.seekg(static_cast<std::streamoff>(fileOffset), std::ios::beg);
    std::string line;
    bool loaded = false;
    while (std::getline(file, line))
    {
        if (line.empty() || line[0] == '#')
        {
            std::streampos pos = file.tellg();
            if (pos != std::streampos(-1))
            {
                fileOffset = static_cast<size_t>(pos);
            }
            continue;
        }
        commandQueue.push_back(line);
        loaded = true;
        std::streampos pos = file.tellg();
        if (pos != std::streampos(-1))
        {
            fileOffset = static_cast<size_t>(pos);
        }
    }
    return loaded;
}

static bool splitLine(const std::string &line, std::vector<std::string> &parts)
{
    parts.clear();
    std::string current;
    for (char ch : line)
    {
        if (ch == '|')
        {
            parts.push_back(current);
            current.clear();
        }
        else
        {
            current.push_back(ch);
        }
    }
    parts.push_back(current);
    return !parts.empty();
}

bool InputHarness::nextInt(const std::string &prompt, int min, int max, int &out)
{
    if (!enabled)
    {
        return false;
    }
    if (nextCommandIndex >= commandQueue.size())
    {
        loadNewCommands();
    }
    if (nextCommandIndex >= commandQueue.size())
    {
        return false;
    }

    const std::string &line = commandQueue[nextCommandIndex++];
    std::vector<std::string> parts;
    if (!splitLine(line, parts) || parts.size() < 3)
    {
        return false;
    }
    if (parts[0] != "input_int")
    {
        return false;
    }
    if (parts[1] != prompt)
    {
        return false;
    }
    int value = 0;
    std::istringstream valueStream(parts[2]);
    if (!(valueStream >> value))
    {
        return false;
    }
    if (value < min || value > max)
    {
        return false;
    }
    out = value;
    return true;
}

bool InputHarness::selectSingle(const std::string &title, const std::vector<std::string> &options, int &outIndex)
{
    if (!enabled)
    {
        return false;
    }
    if (nextCommandIndex >= commandQueue.size())
    {
        loadNewCommands();
    }
    if (nextCommandIndex >= commandQueue.size())
    {
        return false;
    }

    const std::string &line = commandQueue[nextCommandIndex++];
    std::vector<std::string> parts;
    if (!splitLine(line, parts) || parts.size() < 3)
    {
        return false;
    }
    if (parts[0] != "select")
    {
        return false;
    }
    if (parts[1] != title)
    {
        return false;
    }
    int index = -1;
    std::istringstream valueStream(parts[2]);
    if (!(valueStream >> index))
    {
        return false;
    }
    if (index < 0 || static_cast<size_t>(index) >= options.size())
    {
        return false;
    }
    outIndex = index;
    return true;
}

InputHarness *getInputHarness()
{
    return activeHarness;
}

void setInputHarness(InputHarness *harness)
{
    activeHarness = harness;
}
} // namespace ui
#endif
