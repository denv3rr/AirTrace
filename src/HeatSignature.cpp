#include "HeatSignatureAlgorithm.h"
#include <algorithm>

constexpr float MIN_ADJUSTMENT = 0.05f; // Minimum adjustment factor for a single movement

void HeatSignatureAlgorithm::setHeatData(float heat)
{
    heatLevel = heat;
}

std::pair<int, int> HeatSignatureAlgorithm::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    // Adjust path based on heat intensity: where higher heat = closer
    float adjustmentFactor = std::min(std::max(heatLevel / 100.0f, MIN_ADJUSTMENT), 0.05f); // Capped and floored adjustment to 0.05f
                                                                                            // Change as needed

    int newX = static_cast<int>(followerPos.first + adjustmentFactor * (targetPos.first - followerPos.first));
    int newY = static_cast<int>(followerPos.second + adjustmentFactor * (targetPos.second - followerPos.second));

    return {newX, newY};
}
