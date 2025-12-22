#ifndef SIMULATION_UTILS_H
#define SIMULATION_UTILS_H

#include <vector>

#include "core/Object.h"

std::vector<Object> generateTargets(int count = 5, int maxCoord = 100);
float calculateHeatSignature(const Object &source, const Object &target);

#endif // SIMULATION_UTILS_H
