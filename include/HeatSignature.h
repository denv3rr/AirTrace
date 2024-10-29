#ifndef HEATSIGNATURE_H
#define HEATSIGNATURE_H

#include "PathCalculator.h"

class HeatSignature : public PathCalculator
{
public:
    std::tuple<int, int, int> calculatePath(const Object &follower, const Object &target) override;
    void setHeatData(float heatLevel);

private:
    float heatLevel;
};

#endif // HEATSIGNATURE_H