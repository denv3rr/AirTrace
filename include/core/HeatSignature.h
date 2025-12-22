#ifndef HEATSIGNATURE_H
#define HEATSIGNATURE_H

#include "core/PathCalculator.h"

class HeatSignature : public PathCalculator
{
public:
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;
    void setHeatData(float heatLevel);

private:
    float heatLevel = 0.0f;
};

#endif // HEATSIGNATURE_H
