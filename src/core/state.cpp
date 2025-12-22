#include "core/state.h"

State9 integrateState(const State9 &state, double dt)
{
    State9 next = state;
    next.position.x += state.velocity.x * dt + 0.5 * state.acceleration.x * dt * dt;
    next.position.y += state.velocity.y * dt + 0.5 * state.acceleration.y * dt * dt;
    next.position.z += state.velocity.z * dt + 0.5 * state.acceleration.z * dt * dt;

    next.velocity.x += state.acceleration.x * dt;
    next.velocity.y += state.acceleration.y * dt;
    next.velocity.z += state.acceleration.z * dt;

    next.time += dt;
    return next;
}

Projection2D projectXY(const State9 &state)
{
    return {state.position.x, state.position.y, "XY"};
}

Projection2D projectXZ(const State9 &state)
{
    return {state.position.x, state.position.z, "XZ"};
}

Projection2D projectYZ(const State9 &state)
{
    return {state.position.y, state.position.z, "YZ"};
}
