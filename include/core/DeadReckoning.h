#ifndef DEADRECKONING_H
#define DEADRECKONING_H

#include "core/PathCalculator.h"

class DeadReckoning : public PathCalculator
{
public:
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;

private:
    std::pair<int, int> lastKnownPosition{0, 0};
    double speed = 2.0;
    double heading = 45.0; // in degrees
};

#endif // DEADRECKONING_H

