#ifndef TOOLS_ADAPTER_REGISTRY_LOADER_H
#define TOOLS_ADAPTER_REGISTRY_LOADER_H

#include <string>
#include <vector>

#include "core/sim_config.h"

namespace tools
{
struct AdapterUiField
{
    std::string fieldId;
    std::string type;
    std::string units;
    double rangeMin = 0.0;
    double rangeMax = 0.0;
    std::string errorBehavior;
    std::vector<std::string> surfaces;
};

struct AdapterUiSnapshot
{
    std::string adapterId;
    std::string adapterVersion;
    std::string surface;
    std::string status;
    std::string reason;
    std::string approvedBy;
    std::string approvalDate;
    std::string signatureAlgorithm;
    std::string contextVersionSummary;
    std::vector<AdapterUiField> fields;
};

AdapterUiSnapshot loadAdapterUiSnapshot(const SimConfig &config);
bool validateAdapterSelection(const SimConfig &config, std::string &reason);
} // namespace tools

#endif // TOOLS_ADAPTER_REGISTRY_LOADER_H
