#ifndef PATHCALCULATOR_H
#define PATHCALCULATOR_H

#include "core/Object.h"
#include <utility> // For std::pair

class PathCalculator
{
public:
    virtual ~PathCalculator() = default;

    // Pure virtual function (must be implemented by derived classes)
    virtual std::pair<int, int> calculatePath(const Object &follower, const Object &target) = 0;
};

#endif // PATHCALCULATOR_H

