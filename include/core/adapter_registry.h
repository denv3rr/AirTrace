#ifndef CORE_ADAPTER_REGISTRY_H
#define CORE_ADAPTER_REGISTRY_H

#include <string>
#include <vector>

struct AdapterCapability
{
    std::string id;
    std::string description;
    std::string units;
    double rangeMin = 0.0;
    double rangeMax = 0.0;
    std::string errorBehavior;
};

struct AdapterUiExtension
{
    std::string fieldId;
    std::string type;
    std::string units;
    double rangeMin = 0.0;
    double rangeMax = 0.0;
    std::string errorBehavior;
    std::vector<std::string> surfaces;
};

struct AdapterManifest
{
    std::string adapterId;
    std::string adapterVersion;
    std::string adapterContractVersion;
    std::string uiContractVersion;
    std::string coreCompatibilityMin;
    std::string coreCompatibilityMax;
    std::string toolsCompatibilityMin;
    std::string toolsCompatibilityMax;
    std::string uiCompatibilityMin;
    std::string uiCompatibilityMax;
    std::vector<AdapterCapability> capabilities;
    std::vector<AdapterUiExtension> uiExtensions;
};

struct AdapterAllowlistEntry
{
    std::string adapterId;
    std::string adapterVersion;
    std::string signatureHash;
    std::string signatureAlgorithm;
    std::string approvedBy;
    std::string approvalDate;
    std::vector<std::string> allowedSurfaces;
};

struct AdapterRegistryContext
{
    std::string coreVersion;
    std::string toolsVersion;
    std::string uiVersion;
    std::string adapterContractVersion;
    std::string uiContractVersion;
};

struct AdapterRegistryResult
{
    bool ok = false;
    std::string reason;
};

bool validateAdapterRegistration(const AdapterManifest &manifest,
                                 const AdapterAllowlistEntry &allowlist,
                                 const AdapterRegistryContext &context,
                                 const std::string &requestedSurface,
                                 AdapterRegistryResult &result);

#endif // CORE_ADAPTER_REGISTRY_H
