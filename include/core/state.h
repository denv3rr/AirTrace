#ifndef CORE_STATE_H
#define CORE_STATE_H

#include <string>

struct Vec3
{
    double x;
    double y;
    double z;
};

struct State9
{
    Vec3 position;
    Vec3 velocity;
    Vec3 acceleration;
    double time;
};

struct Projection2D
{
    double x;
    double y;
    std::string plane;
};

State9 integrateState(const State9 &state, double dt);
Projection2D projectXY(const State9 &state);
Projection2D projectXZ(const State9 &state);
Projection2D projectYZ(const State9 &state);

#endif // CORE_STATE_H
