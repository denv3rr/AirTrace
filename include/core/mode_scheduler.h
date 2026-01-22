#ifndef CORE_MODE_SCHEDULER_H
#define CORE_MODE_SCHEDULER_H

#include <string>
#include <vector>

#include "core/multi_modal_types.h"

struct PipelineRequest
{
    std::string name;
    ModeType type = ModeType::Primary;
    bool eligible = false;
    bool snapshot = false;
    double estimatedCostMs = 0.0;
    double lastServiceSeconds = 0.0;
};

struct ScheduleResult
{
    std::vector<std::string> scheduled;
    std::vector<std::string> deferred;
};

class ModeScheduler
{
public:
    explicit ModeScheduler(SchedulerConfig config = {});
    ScheduleResult schedule(const std::vector<PipelineRequest> &requests, double nowSeconds) const;

private:
    SchedulerConfig config;
};

#endif // CORE_MODE_SCHEDULER_H
