#ifndef CORE_MOTION_MODELS_H
#define CORE_MOTION_MODELS_H

#include <random>

#include "core/state.h"

enum class MotionModelType
{
    ConstantVelocity,
    ConstantAcceleration,
    CoordinatedTurn,
    RandomManeuver
};

struct MotionBounds
{
    Vec3 minPosition;
    Vec3 maxPosition;
    double maxSpeed;
    double maxAcceleration;
    double maxTurnRateDeg;
};

struct ManeuverParams
{
    double randomAccelStd;
    double maneuverProbability;
};

State9 stepMotionModel(const State9 &state,
                       MotionModelType model,
                       double dt,
                       const MotionBounds &bounds,
                       const ManeuverParams &params,
                       std::mt19937 &rng);

#endif // CORE_MOTION_MODELS_H
