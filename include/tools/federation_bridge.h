#ifndef TOOLS_FEDERATION_BRIDGE_H
#define TOOLS_FEDERATION_BRIDGE_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "core/external_io_envelope.h"

namespace tools
{
struct FederationBridgeConfig
{
    struct EndpointConfig
    {
        std::string endpointId = "endpoint_default";
        std::string outputFormatName = "ie_json_v1";
        bool enabled = true;
    };

    std::uint64_t startLogicalTick = 0;
    std::uint64_t tickStep = 1;
    std::uint64_t startTimestampMs = 0;
    std::uint64_t tickDurationMs = 10;
    double maxLatencyBudgetMs = 250.0;
    bool requireDeterministic = true;
    std::string outputFormatName = "ie_json_v1";
    std::string federateId = "federate_default";
    std::string routeDomain = "mission";
    bool requireSourceTimestamp = true;
    bool requireMonotonicSourceTimestamp = true;
    std::uint64_t maxFutureSkewMs = 0;
    std::vector<std::string> allowedSourceIds{};
    std::string federateKeyId = "key_default";
    std::vector<EndpointConfig> endpoints{};
};

struct FederationEventFrame
{
    std::string schemaVersion = "1.0.0";
    std::string interfaceId = "airtrace.federation_event";
    std::string endpointId;
    std::string federateId;
    std::string federateKeyId;
    std::string routeKey;
    std::uint64_t routeSequence = 0;
    std::uint64_t logicalTick = 0;
    std::uint64_t eventTimestampMs = 0;
    std::uint64_t sourceTimestampMs = 0;
    double sourceLatencyMs = 0.0;
    double latencyBudgetMs = 0.0;
    std::string sourceId;
    std::string payloadFormat;
    std::string payload;
    unsigned int seed = 0;
    bool deterministic = true;
};

struct FederationBridgeResult
{
    bool ok = false;
    std::string error;
    FederationEventFrame frame{};
};

struct FederationFanoutResult
{
    bool ok = false;
    std::string error;
    std::vector<FederationEventFrame> frames{};
};

class FederationBridge
{
public:
    explicit FederationBridge(FederationBridgeConfig config);
    FederationBridgeResult publish(const ExternalIoEnvelope &envelope);
    FederationFanoutResult publishFanout(const ExternalIoEnvelope &envelope);

private:
    FederationBridgeConfig config_{};
    std::uint64_t nextLogicalTick_ = 0;
    std::vector<std::string> allowedSourcesNormalized_{};
    std::unordered_map<std::string, std::uint64_t> routeSequenceByKeyAndEndpoint_{};
    std::unordered_map<std::string, std::uint64_t> lastSourceTimestampByKey_{};
};

std::string serializeFederationEventFrameJson(const FederationEventFrame &frame);
} // namespace tools

#endif // TOOLS_FEDERATION_BRIDGE_H
