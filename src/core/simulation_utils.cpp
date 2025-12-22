#include "core/simulation_utils.h"

#include <cstdlib>
#include <cmath>
#include <string>

std::vector<Object> generateTargets(int count, int maxCoord)
{
    std::vector<Object> targets;
    targets.reserve(static_cast<size_t>(count));

    for (int i = 0; i < count; ++i)
    {
        Object target(i + 1, "Target_" + std::to_string(i + 1),
                      {std::rand() % maxCoord, std::rand() % maxCoord});
        targets.push_back(target);
    }

    return targets;
}

float calculateHeatSignature(const Object &source, const Object &target)
{
    float distance = std::sqrt(std::pow(target.getPosition().first - source.getPosition().first, 2) +
                               std::pow(target.getPosition().second - source.getPosition().second, 2));
    return source.getHeatLevel() / (1.0f + std::pow(distance, 2));
}
