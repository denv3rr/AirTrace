#ifndef CORE_MULTI_MODAL_TYPES_H
#define CORE_MULTI_MODAL_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

enum class ModeType
{
    Primary,
    Fused,
    AuxSnapshot
};

struct ModeDefinition
{
    std::string name;
    std::vector<std::string> requiredSensors;
    std::vector<std::string> optionalSensors;
    ModeType type = ModeType::Primary;
};

struct SourceStatus
{
    std::string name;
    bool health = false;
    double dataAgeSeconds = 0.0;
    double confidence = 0.0;
    bool policyAllowed = false;
    std::uint64_t lastUpdateTick = 0;
};

struct ModeDecisionDetail
{
    std::string selectedMode;
    std::vector<std::string> contributors;
    double confidence = 0.0;
    std::string reason;
    std::string downgradeReason;
};

struct PipelineBudget
{
    double reservedMs = 0.0;
    std::size_t maxOutstanding = 0;
    double minServiceIntervalSeconds = 0.0;
};

struct SchedulerConfig
{
    double primaryBudgetMs = 5.0;
    double auxBudgetMs = 2.0;
    std::size_t maxAuxPipelines = 2;
    double auxMinServiceIntervalSeconds = 1.0;
    bool allowSnapshotOverlap = true;
};

#endif // CORE_MULTI_MODAL_TYPES_H
