#ifndef INPUT_HARNESS_H
#define INPUT_HARNESS_H

#include <string>
#include <vector>

namespace ui
{
#if defined(AIRTRACE_TEST_HARNESS)
class InputHarness
{
public:
    explicit InputHarness(std::string commandPath);

    bool isEnabled() const;
    bool nextInt(const std::string &prompt, int min, int max, int &out);
    bool selectSingle(const std::string &title, const std::vector<std::string> &options, int &outIndex);

private:
    bool loadNewCommands();

    std::string commandPath;
    std::vector<std::string> commandQueue;
    size_t nextCommandIndex = 0;
    size_t fileOffset = 0;
    bool enabled = false;
};

InputHarness *getInputHarness();
void setInputHarness(InputHarness *harness);
#else
class InputHarness
{
public:
    explicit InputHarness(std::string) {}
    bool isEnabled() const { return false; }
    bool nextInt(const std::string &, int, int, int &) { return false; }
    bool selectSingle(const std::string &, const std::vector<std::string> &, int &) { return false; }
};

inline InputHarness *getInputHarness()
{
    return nullptr;
}
inline void setInputHarness(InputHarness *) {}
#endif
} // namespace ui

#endif // INPUT_HARNESS_H
