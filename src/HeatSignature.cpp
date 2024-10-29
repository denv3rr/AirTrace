#include "HeatSignature.h"
#include <algorithm>

constexpr float MIN_ADJUSTMENT = 0.05f; // Minimum adjustment factor for a single movement

void HeatSignature::setHeatData(float heat)
{
    float heatLevel = static_cast<float>(heat);
}

std::tuple<int, int, int> HeatSignature::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    // Adjust path based on heat intensity: where higher heat = closer
    float adjustmentFactor = std::clamp(heatLevel / 100.0f, MIN_ADJUSTMENT, 0.05f); // Capped and floored adjustment to 0.05f
                                                                                    // Change as needed

    int newX = static_cast<int>(followerPos.first + adjustmentFactor * (targetPos.first - followerPos.first));
    int newY = static_cast<int>(followerPos.second + adjustmentFactor * (targetPos.second - followerPos.second));
    int newZ = static_cast<int>(followerPos.third + adjustmentFactor * (targetPos.third - followerPos.third));

    return {newX, newY, newZ};
}
