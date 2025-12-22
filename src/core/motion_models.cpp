#include "core/motion_models.h"

#include <algorithm>
#include <cmath>

namespace
{
double clampValue(double value, double minValue, double maxValue)
{
    return std::max(minValue, std::min(value, maxValue));
}

double magnitude(const Vec3 &v)
{
    return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 clampVectorMagnitude(const Vec3 &v, double maxMagnitude)
{
    double mag = magnitude(v);
    if (mag <= maxMagnitude || mag == 0.0)
    {
        return v;
    }
    double scale = maxMagnitude / mag;
    return {v.x * scale, v.y * scale, v.z * scale};
}

Vec3 applyTurn(const Vec3 &velocity, double turnRateDeg, double dt)
{
    double speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
    if (speed == 0.0)
    {
        return velocity;
    }
    double turnRateRad = turnRateDeg * (3.14159265358979323846 / 180.0);
    double heading = std::atan2(velocity.y, velocity.x);
    double newHeading = heading + turnRateRad * dt;
    return {speed * std::cos(newHeading), speed * std::sin(newHeading), velocity.z};
}

State9 clampState(const State9 &state, const MotionBounds &bounds)
{
    State9 next = state;
    next.velocity = clampVectorMagnitude(next.velocity, bounds.maxSpeed);
    next.acceleration = clampVectorMagnitude(next.acceleration, bounds.maxAcceleration);

    next.position.x = clampValue(next.position.x, bounds.minPosition.x, bounds.maxPosition.x);
    next.position.y = clampValue(next.position.y, bounds.minPosition.y, bounds.maxPosition.y);
    next.position.z = clampValue(next.position.z, bounds.minPosition.z, bounds.maxPosition.z);
    return next;
}
} // namespace

State9 stepMotionModel(const State9 &state,
                       MotionModelType model,
                       double dt,
                       const MotionBounds &bounds,
                       const ManeuverParams &params,
                       std::mt19937 &rng)
{
    State9 next = state;
    std::normal_distribution<double> accelNoise(0.0, params.randomAccelStd);
    std::bernoulli_distribution maneuverChance(params.maneuverProbability);

    switch (model)
    {
    case MotionModelType::ConstantVelocity:
        next.acceleration = {0.0, 0.0, 0.0};
        break;
    case MotionModelType::ConstantAcceleration:
        break;
    case MotionModelType::CoordinatedTurn:
        next.velocity = applyTurn(state.velocity, bounds.maxTurnRateDeg, dt);
        next.acceleration = {0.0, 0.0, 0.0};
        break;
    case MotionModelType::RandomManeuver:
        if (maneuverChance(rng))
        {
            next.acceleration = {accelNoise(rng), accelNoise(rng), accelNoise(rng)};
        }
        break;
    default:
        break;
    }

    next = integrateState(next, dt);
    return clampState(next, bounds);
}
