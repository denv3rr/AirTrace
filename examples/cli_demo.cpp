#include <iostream>
#include <string>

#include "core/Object.h"
#include "core/Tracker.h"

namespace
{
void runMode(const std::string &mode, int steps)
{
    Object target(1, "Target", std::pair<int, int>{20, 20});
    Object follower(2, "Follower", std::pair<int, int>{0, 0});

    Tracker tracker(follower);
    tracker.setTrackingMode(mode);
    tracker.setTarget(target);

    std::cout << "\n== " << mode << " ==\n";
    for (int i = 0; i < steps && tracker.isTrackingActive(); ++i)
    {
        auto targetPos = target.getPosition();
        target.moveTo({targetPos.first + 1, targetPos.second + 1});

        if (mode == "heat_signature")
        {
            tracker.updateHeatSignature(50.0f);
        }

        tracker.update();
    }
}
} // namespace

int main()
{
    runMode("prediction", 3);
    runMode("kalman", 3);
    runMode("heat_signature", 3);
    runMode("gps", 3);
    runMode("dead_reckoning", 3);

    std::cout << "\nExample run complete.\n";
    return 0;
}
