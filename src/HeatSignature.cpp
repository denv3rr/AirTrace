#include "HeatSignatureAlgorithm.h"

void HeatSignatureAlgorithm::setHeatData(float heat)
{
    heatLevel = heat;
}

std::pair<int, int> HeatSignatureAlgorithm::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    // Adjust path based on heat intensity: higher heat = closer tracking
    float adjustmentFactor = (heatLevel > 50.0) ? 0.8f : 0.5f; // Set current threshold

    int newX = static_cast<int>(followerPos.first + adjustmentFactor * (targetPos.first - followerPos.first));
    int newY = static_cast<int>(followerPos.second + adjustmentFactor * (targetPos.second - followerPos.second));

    return {newX, newY};
}
