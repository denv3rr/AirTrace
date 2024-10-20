#ifndef DEADRECKONING_H
#define DEADRECKONING_H

#include "PathCalculator.h"

class DeadReckoning : public PathCalculator
{
public:
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;

private:
    std::pair<int, int> lastKnownPosition;
    double speed;
    double heading; // in degrees
};

#endif // DEADRECKONING_H
