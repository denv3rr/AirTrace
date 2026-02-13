#ifndef CORE_EXTERNAL_IO_ENVELOPE_H
#define CORE_EXTERNAL_IO_ENVELOPE_H

#include <string>
#include <vector>

// Versioned, deterministic envelope for cross-platform integration.
struct ExternalIoMetadata
{
    std::string schemaVersion = "1.0.0";
    std::string interfaceId = "airtrace.external_io";
    std::string platformProfile;
    std::string adapterId;
    std::string adapterVersion;
    std::string uiSurface;
    unsigned int seed = 0;
    bool deterministic = true;
};

struct ExternalIoSensorRecord
{
    std::string sensorId;
    bool available = false;
    bool healthy = false;
    bool hasMeasurement = false;
    double freshnessSeconds = 0.0;
    double confidence = 0.0;
    std::string lastError;
};

struct ExternalIoModeRecord
{
    std::string activeMode;
    std::vector<std::string> contributors;
    double confidence = 0.0;
    std::string decisionReason;
    std::string denialReason;
    std::string ladderStatus;
};

struct ExternalIoFrontViewRecord
{
    std::string activeMode;
    std::string viewState;
    std::string frameId;
    std::string sensorType;
    unsigned int sequence = 0;
    double latencyMs = 0.0;
    int droppedFrames = 0;
    std::string dropReason;
    bool spoofActive = false;
    double confidence = 0.0;
    std::string provenance;
    std::string authStatus;
};

struct ExternalIoEnvelope
{
    ExternalIoMetadata metadata{};
    std::vector<ExternalIoSensorRecord> sensors{};
    ExternalIoModeRecord mode{};
    std::string disqualifiedSources;
    std::string lockoutStatus;
    std::string authStatus;
    std::string provenanceStatus;
    std::string loggingStatus;
    std::string adapterStatus;
    std::string adapterReason;
    std::string adapterFields;
    ExternalIoFrontViewRecord frontView{};
};

#endif // CORE_EXTERNAL_IO_ENVELOPE_H
