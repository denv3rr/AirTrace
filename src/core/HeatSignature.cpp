#include "core/HeatSignature.h"
#include <algorithm>
#include <cmath>

constexpr float MIN_ADJUSTMENT = 0.05f;
constexpr float MAX_ADJUSTMENT = 0.50f;

void HeatSignature::setHeatData(float heat)
{
    if (!std::isfinite(heat))
    {
        heatLevel = 0.0f;
        return;
    }
    heatLevel = std::clamp(heat, 0.0f, 100.0f);
}

std::pair<int, int> HeatSignature::calculatePath(const Object &follower, const Object &target)
{
    auto targetPos = target.getPosition();
    auto followerPos = follower.getPosition();

    // Movement response is bounded and monotonic with validated heat input.
    float adjustmentFactor = std::clamp(heatLevel / 100.0f, MIN_ADJUSTMENT, MAX_ADJUSTMENT);

    int newX = static_cast<int>(followerPos.first + adjustmentFactor * (targetPos.first - followerPos.first));
    int newY = static_cast<int>(followerPos.second + adjustmentFactor * (targetPos.second - followerPos.second));

    return {newX, newY};
}

