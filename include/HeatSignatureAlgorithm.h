#ifndef HEATSIGNATUREALGORITHM_H
#define HEATSIGNATUREALGORITHM_H

#include "PathCalculator.h"

class HeatSignatureAlgorithm : public PathCalculator
{
public:
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;
    void setHeatData(float heatLevel);

private:
    float heatLevel;
};

#endif // HEATSIGNATUREALGORITHM_H
