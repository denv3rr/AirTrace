#include "core/mode_scheduler.h"

namespace
{
bool isPrimaryType(ModeType type)
{
    return type == ModeType::Primary || type == ModeType::Fused;
}
} // namespace

ModeScheduler::ModeScheduler(SchedulerConfig config)
    : config(config)
{
}

ScheduleResult ModeScheduler::schedule(const std::vector<PipelineRequest> &requests, double nowSeconds) const
{
    ScheduleResult result;
    bool primaryScheduled = false;
    std::size_t auxScheduled = 0;

    for (const auto &request : requests)
    {
        if (!request.eligible || !isPrimaryType(request.type))
        {
            continue;
        }
        if (request.estimatedCostMs > config.primaryBudgetMs)
        {
            result.deferred.push_back(request.name);
            continue;
        }
        result.scheduled.push_back(request.name);
        primaryScheduled = true;
        break;
    }

    for (const auto &request : requests)
    {
        if (!request.eligible || request.type != ModeType::AuxSnapshot)
        {
            continue;
        }
        if (auxScheduled >= config.maxAuxPipelines)
        {
            result.deferred.push_back(request.name);
            continue;
        }
        if (primaryScheduled && !config.allowSnapshotOverlap)
        {
            result.deferred.push_back(request.name);
            continue;
        }
        if (request.estimatedCostMs > config.auxBudgetMs)
        {
            result.deferred.push_back(request.name);
            continue;
        }
        double sinceLast = nowSeconds - request.lastServiceSeconds;
        if (sinceLast < config.auxMinServiceIntervalSeconds)
        {
            result.deferred.push_back(request.name);
            continue;
        }
        result.scheduled.push_back(request.name);
        auxScheduled += 1;
    }

    return result;
}
