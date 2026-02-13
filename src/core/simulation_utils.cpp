#include "core/simulation_utils.h"

#include <cmath>
#include <random>
#include <string>

std::vector<Object> generateTargets(int count, int maxCoord, unsigned int seed)
{
    std::vector<Object> targets;
    if (count <= 0 || maxCoord <= 0)
    {
        return targets;
    }

    targets.reserve(static_cast<size_t>(count));
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> coordinate(0, maxCoord - 1);

    for (int i = 0; i < count; ++i)
    {
        Object target(i + 1, "Target_" + std::to_string(i + 1),
                      {coordinate(rng), coordinate(rng)});
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
