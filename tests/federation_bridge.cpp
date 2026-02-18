#include "tools/audit_log.h"
#include "tools/federation_bridge.h"
#include "tools/io_packager.h"

#include <cassert>
#include <cstdio>
#include <exception>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>

namespace
{
ExternalIoEnvelope makeEnvelope()
{
    ExternalIoEnvelope envelope;
    envelope.metadata.schemaVersion = "1.0.0";
    envelope.metadata.interfaceId = "airtrace.external_io";
    envelope.metadata.platformProfile = "air";
    envelope.metadata.adapterId = "official.air";
    envelope.metadata.adapterVersion = "1.0.0";
    envelope.metadata.uiSurface = "tui";
    envelope.metadata.seed = 42U;
    envelope.metadata.deterministic = true;
    envelope.mode.activeMode = "gps";
    envelope.mode.contributors = {"gps", "imu"};
    envelope.mode.confidence = 0.91;
    envelope.mode.decisionReason = "gps_eligible";
    envelope.mode.denialReason.clear();
    envelope.mode.ladderStatus = "ok";
    envelope.sensors.push_back({"gps", true, true, true, 0.1, 0.95, ""});
    envelope.frontView.sourceId = "front_sensor";
    envelope.frontView.sensorType = "eo";
    envelope.frontView.streamId = "primary";
    envelope.frontView.timestampMs = 1100U;
    envelope.frontView.latencyMs = 10.0;
    envelope.frontView.confidence = 0.80;
    return envelope;
}
} // namespace

int main()
{
    try
    {
        ExternalIoEnvelope envelope = makeEnvelope();

    tools::FederationBridgeConfig bridgeConfig;
    bridgeConfig.startLogicalTick = 100;
    bridgeConfig.tickStep = 5;
    bridgeConfig.startTimestampMs = 1000;
    bridgeConfig.tickDurationMs = 20;
    bridgeConfig.maxLatencyBudgetMs = 2500.0;
    bridgeConfig.requireDeterministic = true;
    bridgeConfig.outputFormatName = "ie_json_v1";
    bridgeConfig.federateId = "edge_node_1";
    bridgeConfig.routeDomain = "theater_alpha";
    bridgeConfig.requireSourceTimestamp = true;
    bridgeConfig.requireMonotonicSourceTimestamp = true;
    bridgeConfig.maxFutureSkewMs = 0;
    bridgeConfig.federateKeyId = "key_alpha";
    bridgeConfig.federateKeyEpoch = 7U;
    bridgeConfig.federateKeyValidFromTimestampMs = 0U;
    bridgeConfig.federateKeyValidUntilTimestampMs = std::numeric_limits<std::uint64_t>::max();
    bridgeConfig.requireFederateAttestation = false;

    tools::FederationBridge bridge(bridgeConfig);

    tools::FederationBridgeResult bridgeFrame1 = bridge.publish(envelope);
    assert(bridgeFrame1.ok);
    assert(bridgeFrame1.frame.logicalTick == 100U);
    assert(bridgeFrame1.frame.eventTimestampMs == 3000U);
    assert(bridgeFrame1.frame.sourceTimestampMs == 1100U);
    assert(bridgeFrame1.frame.sourceLatencyMs == 1900.0);
    assert(bridgeFrame1.frame.federateId == "edge_node_1");
    assert(bridgeFrame1.frame.federateKeyId == "key_alpha");
    assert(bridgeFrame1.frame.federateKeyEpoch == 7U);
    assert(bridgeFrame1.frame.federateKeyValidUntilTimestampMs == std::numeric_limits<std::uint64_t>::max());
    assert(bridgeFrame1.frame.endpointId == "endpoint_default");
    assert(bridgeFrame1.frame.routeKey == "theater_alpha/air/front_sensor");
    assert(bridgeFrame1.frame.routeSequence == 0U);
    assert(bridgeFrame1.frame.payloadFormat == "ie_json_v1");

    tools::IoEnvelopeParseResult bridgePayloadParsed =
        tools::parseExternalIoEnvelope(bridgeFrame1.frame.payloadFormat, bridgeFrame1.frame.payload);
    assert(bridgePayloadParsed.ok);
    assert(bridgePayloadParsed.envelope.mode.activeMode == envelope.mode.activeMode);

    const std::string bridgeJson = tools::serializeFederationEventFrameJson(bridgeFrame1.frame);
    assert(bridgeJson.find("\"logical_tick\":100") != std::string::npos);
    assert(bridgeJson.find("\"payload_format\":\"ie_json_v1\"") != std::string::npos);
    assert(bridgeJson.find("\"endpoint_id\":\"endpoint_default\"") != std::string::npos);
    assert(bridgeJson.find("\"federate_key_epoch\":7") != std::string::npos);

    tools::FederationBridgeResult bridgeFrame2 = bridge.publish(envelope);
    assert(bridgeFrame2.ok);
    assert(bridgeFrame2.frame.logicalTick == 105U);
    assert(bridgeFrame2.frame.eventTimestampMs == 3100U);
    assert(bridgeFrame2.frame.routeSequence == 1U);

    tools::FederationBridgeConfig unsupportedCodecConfig = bridgeConfig;
    unsupportedCodecConfig.outputFormatName = "yaml";
    tools::FederationBridge unsupportedBridge(unsupportedCodecConfig);
    tools::FederationBridgeResult unsupportedCodecResult = unsupportedBridge.publish(envelope);
    assert(!unsupportedCodecResult.ok);

    tools::FederationBridgeConfig invalidEndpointCodecConfig = bridgeConfig;
    invalidEndpointCodecConfig.endpoints = {{"edge_a", "yaml", true}};
    tools::FederationBridge invalidEndpointCodecBridge(invalidEndpointCodecConfig);
    tools::FederationBridgeResult invalidEndpointCodecResult = invalidEndpointCodecBridge.publish(envelope);
    assert(!invalidEndpointCodecResult.ok);

    tools::FederationBridgeConfig nondeterministicRequiredConfig = bridgeConfig;
    nondeterministicRequiredConfig.requireDeterministic = true;
    tools::FederationBridge nondeterministicBridge(nondeterministicRequiredConfig);
    ExternalIoEnvelope nondeterministicEnvelope = envelope;
    nondeterministicEnvelope.metadata.deterministic = false;
    tools::FederationBridgeResult nondeterministicResult = nondeterministicBridge.publish(nondeterministicEnvelope);
    assert(!nondeterministicResult.ok);

    tools::FederationBridgeConfig lowLatencyConfig = bridgeConfig;
    lowLatencyConfig.maxLatencyBudgetMs = 10.0;
    tools::FederationBridge lowLatencyBridge(lowLatencyConfig);
    tools::FederationBridgeResult lowLatencyResult = lowLatencyBridge.publish(envelope);
    assert(!lowLatencyResult.ok);

    tools::FederationBridgeConfig invalidTickConfig = bridgeConfig;
    invalidTickConfig.tickStep = 0;
    tools::FederationBridge invalidTickBridge(invalidTickConfig);
    tools::FederationBridgeResult invalidTickResult = invalidTickBridge.publish(envelope);
    assert(!invalidTickResult.ok);

    tools::FederationBridgeConfig invalidKeyWindowConfig = bridgeConfig;
    invalidKeyWindowConfig.federateKeyValidFromTimestampMs = 5000U;
    invalidKeyWindowConfig.federateKeyValidUntilTimestampMs = 4000U;
    tools::FederationBridge invalidKeyWindowBridge(invalidKeyWindowConfig);
    tools::FederationBridgeResult invalidKeyWindowResult = invalidKeyWindowBridge.publish(envelope);
    assert(!invalidKeyWindowResult.ok);

    tools::FederationBridgeConfig expiredKeyConfig = bridgeConfig;
    expiredKeyConfig.federateKeyValidUntilTimestampMs = 2000U;
    tools::FederationBridge expiredKeyBridge(expiredKeyConfig);
    tools::FederationBridgeResult expiredKeyResult = expiredKeyBridge.publish(envelope);
    assert(!expiredKeyResult.ok);

    tools::FederationBridgeConfig allowedSourceConfig = bridgeConfig;
    allowedSourceConfig.allowedSourceIds = {"front_sensor"};
    tools::FederationBridge allowedSourceBridge(allowedSourceConfig);
    tools::FederationBridgeResult allowedSourceResult = allowedSourceBridge.publish(envelope);
    assert(allowedSourceResult.ok);
    allowedSourceConfig.allowedSourceIds = {"other_source"};
    tools::FederationBridge deniedSourceBridge(allowedSourceConfig);
    tools::FederationBridgeResult deniedSourceResult = deniedSourceBridge.publish(envelope);
    assert(!deniedSourceResult.ok);

    tools::FederationBridgeConfig missingSourceTimestampConfig = bridgeConfig;
    missingSourceTimestampConfig.requireSourceTimestamp = true;
    tools::FederationBridge missingSourceTimestampBridge(missingSourceTimestampConfig);
    ExternalIoEnvelope missingSourceTimestampEnvelope = envelope;
    missingSourceTimestampEnvelope.frontView.timestampMs = 0U;
    tools::FederationBridgeResult missingSourceTimestampResult =
        missingSourceTimestampBridge.publish(missingSourceTimestampEnvelope);
    assert(!missingSourceTimestampResult.ok);

    tools::FederationBridgeConfig futureSkewConfig = bridgeConfig;
    futureSkewConfig.maxFutureSkewMs = 50U;
    tools::FederationBridge futureSkewBridge(futureSkewConfig);
    ExternalIoEnvelope futureSkewEnvelope = envelope;
    futureSkewEnvelope.frontView.timestampMs = 3100U;
    tools::FederationBridgeResult futureSkewResult = futureSkewBridge.publish(futureSkewEnvelope);
    assert(!futureSkewResult.ok);

    tools::FederationBridgeConfig monotonicConfig = bridgeConfig;
    monotonicConfig.maxFutureSkewMs = 1000U;
    tools::FederationBridge monotonicBridge(monotonicConfig);
    ExternalIoEnvelope monotonicEnvelope = envelope;
    monotonicEnvelope.frontView.timestampMs = 1050U;
    tools::FederationBridgeResult monotonicFirst = monotonicBridge.publish(monotonicEnvelope);
    assert(monotonicFirst.ok);
    monotonicEnvelope.frontView.timestampMs = 1040U;
    tools::FederationBridgeResult monotonicSecond = monotonicBridge.publish(monotonicEnvelope);
    assert(!monotonicSecond.ok);

    tools::FederationBridgeConfig fanoutConfig = bridgeConfig;
    fanoutConfig.startLogicalTick = 0;
    fanoutConfig.tickStep = 1;
    fanoutConfig.startTimestampMs = 0;
    fanoutConfig.tickDurationMs = 10;
    fanoutConfig.maxLatencyBudgetMs = 1000.0;
    fanoutConfig.maxFutureSkewMs = 1000U;
    fanoutConfig.requireSourceTimestamp = false;
    fanoutConfig.requireFederateAttestation = true;
    fanoutConfig.federateAttestationTag = "attest_alpha";
    fanoutConfig.endpoints = {
        {"edge_a", "ie_json_v1", true, true, {"key_alpha"}},
        {"edge_b", "ie_kv_v1", true, true, {"key_alpha"}}};
    tools::FederationBridge fanoutBridge(fanoutConfig);
    ExternalIoEnvelope fanoutEnvelope = envelope;
    fanoutEnvelope.frontView.timestampMs = 0U;
    tools::FederationFanoutResult fanoutResult = fanoutBridge.publishFanout(fanoutEnvelope);
    assert(fanoutResult.ok);
    assert(fanoutResult.frames.size() == 2U);
    assert(fanoutResult.frames[0].routeSequence == 0U);
    assert(fanoutResult.frames[1].routeSequence == 0U);
    assert(fanoutResult.frames[0].endpointId == "edge_a");
    assert(fanoutResult.frames[1].endpointId == "edge_b");
    assert(fanoutResult.frames[0].federateAttestationTag == "attest_alpha");
    assert(fanoutResult.frames[0].federateKeyId == "key_alpha");
    assert(fanoutResult.frames[0].federateKeyEpoch == 7U);
    tools::IoEnvelopeParseResult fanoutParsedA =
        tools::parseExternalIoEnvelope(fanoutResult.frames[0].payloadFormat, fanoutResult.frames[0].payload);
    assert(fanoutParsedA.ok);
    tools::IoEnvelopeParseResult fanoutParsedB =
        tools::parseExternalIoEnvelope(fanoutResult.frames[1].payloadFormat, fanoutResult.frames[1].payload);
    assert(fanoutParsedB.ok);
    tools::FederationFanoutResult fanoutResult2 = fanoutBridge.publishFanout(fanoutEnvelope);
    assert(fanoutResult2.ok);
    assert(fanoutResult2.frames[0].routeSequence == 1U);
    assert(fanoutResult2.frames[1].routeSequence == 1U);

    tools::FederationBridgeConfig untrustedKeyFanoutConfig = fanoutConfig;
    untrustedKeyFanoutConfig.endpoints = {
        {"edge_a", "ie_json_v1", true, true, {"key_other"}}};
    tools::FederationBridge untrustedKeyFanoutBridge(untrustedKeyFanoutConfig);
    tools::FederationFanoutResult untrustedKeyFanoutResult = untrustedKeyFanoutBridge.publishFanout(fanoutEnvelope);
    assert(!untrustedKeyFanoutResult.ok);

    tools::FederationBridgeConfig missingAttestationConfig = fanoutConfig;
    missingAttestationConfig.federateAttestationTag.clear();
    tools::FederationBridge missingAttestationBridge(missingAttestationConfig);
    tools::FederationFanoutResult missingAttestationResult = missingAttestationBridge.publishFanout(fanoutEnvelope);
    assert(!missingAttestationResult.ok);

    std::filesystem::path auditLogPath = std::filesystem::current_path() / "audit_log_federation_test.jsonl";
    std::error_code removeError;
    (void)std::filesystem::remove(auditLogPath, removeError);
    tools::AuditLogConfig auditConfig;
    auditConfig.logPath = auditLogPath.string();
    auditConfig.configPath = "configs/sim_default.cfg";
    auditConfig.buildId = "test";
    auditConfig.role = "test";
    auditConfig.configVersion = "1.0";
    auditConfig.seed = 42U;
    std::string auditStatus;
    bool auditInitOk = tools::initializeAuditLog(auditConfig, auditStatus);
    assert(auditInitOk);
    tools::FederationBridgeResult auditedDenialResult = deniedSourceBridge.publish(envelope);
    assert(!auditedDenialResult.ok);
    std::string auditContent;
    {
        std::ifstream auditFile(auditLogPath.string());
        auditContent.assign(std::istreambuf_iterator<char>(auditFile), std::istreambuf_iterator<char>());
    }
    assert(auditContent.find("\"event\":\"federation_bridge_denied\"") != std::string::npos);
    assert(auditContent.find("source not allowed by route policy") != std::string::npos);
    removeError.clear();
    (void)std::filesystem::remove(auditLogPath, removeError);

        return 0;
    }
    catch (const std::exception &ex)
    {
        std::fprintf(stderr, "exception: %s\n", ex.what());
        return 1;
    }
}
