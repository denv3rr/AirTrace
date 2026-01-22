#include "ui/input_harness.h"

#include <cassert>
#include <cstdlib>

int main()
{
#if defined(_WIN32)
    _putenv("AIRTRACE_TEST_HARNESS=");
#else
    unsetenv("AIRTRACE_TEST_HARNESS");
#endif
    ui::InputHarness harness("configs/harness_commands.txt");
    assert(!harness.isEnabled());
    return 0;
}
