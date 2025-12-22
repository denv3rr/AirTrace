#include "core/DeadReckoning.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::pair<int, int> DeadReckoning::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();

    if (lastKnownPosition == std::pair<int, int>())
    {
        lastKnownPosition = targetPos;
    }

    // Estimate the target's movement using speed and heading
    double radians = heading * M_PI / 180.0;
    int newX = static_cast<int>(lastKnownPosition.first + speed * cos(radians));
    int newY = static_cast<int>(lastKnownPosition.second + speed * sin(radians));

    lastKnownPosition = {newX, newY};
    return lastKnownPosition;
}

