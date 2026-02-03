#ifndef ADAPTER_SDK_H
#define ADAPTER_SDK_H

#include <string>
#include <vector>

#include "core/provenance.h"
#include "core/sensors.h"

namespace adapter
{
constexpr const char *kAdapterContractVersion = "1.0.0";
constexpr const char *kUiContractVersion = "1.0.0";

struct AdapterIdentity
{
    std::string id;
    std::string version;
    std::string contractVersion;
    std::string uiContractVersion;
};

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
    AdapterIdentity identity;
    std::vector<AdapterCapability> capabilities;
    std::vector<AdapterUiExtension> uiExtensions;
};

struct AdapterMeasurement
{
    std::string sensorId;
    Measurement measurement;
    SensorStatus status;
};

class AdapterModule
{
public:
    virtual ~AdapterModule() = default;
    virtual AdapterManifest manifest() const = 0;
    virtual bool initialize(std::string &status) = 0;
    virtual void shutdown() = 0;
    virtual std::vector<AdapterMeasurement> pollMeasurements(double dtSeconds) = 0;
};
} // namespace adapter

#endif // ADAPTER_SDK_H
