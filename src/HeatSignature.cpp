#include "HeatSignatureAlgorithm.h"

void HeatSignatureAlgorithm::setHeatData(float heat)
{
    heatLevel = heat;
}

std::pair<int, int> HeatSignatureAlgorithm::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    // Adjust path based on heat intensity: where higher heat = closer
    float adjustmentFactor = std::min(heatLevel / 100.0f, 0.5f); // Cap the adjustment to avoid overshooting goal

    int newX = static_cast<int>(followerPos.first + adjustmentFactor * (targetPos.first - followerPos.first));
    int newY = static_cast<int>(followerPos.second + adjustmentFactor * (targetPos.second - followerPos.second));

    return {newX, newY};
}
