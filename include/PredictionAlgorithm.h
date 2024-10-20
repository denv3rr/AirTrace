#ifndef PREDICTIONALGORITHM_H
#define PREDICTIONALGORITHM_H

#include "PathCalculator.h"

class PredictionAlgorithm : public PathCalculator
{
public:
    std::pair<int, int> calculatePath(const Object &follower, const Object &target) override;

private:
    std::pair<int, int> predictFuturePosition(const Object &target);
};

#endif // PREDICTIONALGORITHM_H
