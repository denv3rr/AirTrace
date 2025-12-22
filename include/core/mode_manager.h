#ifndef CORE_MODE_MANAGER_H
#define CORE_MODE_MANAGER_H

#include <string>
#include <vector>

#include "core/sensors.h"

enum class TrackingMode
{
    Gps,
    Thermal,
    Radar,
    DeadReckoning,
    Inertial,
    Hold
};

struct ModeDecision
{
    TrackingMode mode;
    std::string reason;
};

class ModeManager
{
public:
    ModeDecision decide(const std::vector<SensorBase *> &sensors) const;
    static std::string modeName(TrackingMode mode);
};

#endif // CORE_MODE_MANAGER_H
