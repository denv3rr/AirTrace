#include "core/mode_manager.h"
#include "core/sensors.h"

#include <cassert>
#include <cmath>
#include <vector>

namespace
{
class TestSensor : public SensorBase
{
public:
    explicit TestSensor(std::string name)
        : SensorBase(std::move(name), {1.0, 0.0, 0.0, 0.0, 1.0})
    {
    }

    void setProvenance(ProvenanceTag tag)
    {
        provenance = tag;
    }

    void setStatus(bool healthy, double ageSeconds, double confidence)
    {
        status.healthy = healthy;
        status.timeSinceLastValid = ageSeconds;
        status.confidence = confidence;
        status.hasMeasurement = healthy;
        status.lastMeasurement.valid = healthy;
        status.lastMeasurement.provenance = provenance;
    }

    void setPositionMeasurement(const Vec3 &position, double timeSeconds)
    {
        Measurement measurement;
        measurement.position = position;
        measurement.valid = true;
        measurement.provenance = provenance;
        status.hasMeasurement = true;
        status.lastMeasurement = measurement;
        status.lastMeasurementTime = timeSeconds;
    }

protected:
    Measurement generateMeasurement(const State9 &, std::mt19937 &) override
    {
        Measurement measurement;
        measurement.valid = true;
        return measurement;
    }

private:
    ProvenanceTag provenance = ProvenanceTag::Operational;
};
} // namespace

int main()
{
    ModeManagerConfig jitterConfig;
    jitterConfig.minHealthyCount = 1;
    jitterConfig.maxDataAgeSeconds = 0.2;
    jitterConfig.maxStaleCount = 2;
    jitterConfig.lockoutSteps = 2;
    jitterConfig.historyWindow = 3;
    jitterConfig.permittedSensors = {"gps"};
    jitterConfig.ladderOrder = {"gps"};
    ModeManager jitterManager(jitterConfig);
    TestSensor jitterGps("gps");
    std::vector<SensorBase *> jitterSensors{&jitterGps};

    jitterGps.setStatus(true, 0.3, 0.9);
    ModeDecision decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.25, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Hold);

    jitterGps.setStatus(true, 0.1, 0.9);
    decision = jitterManager.decide(jitterSensors);
    assert(decision.mode == TrackingMode::Gps);

    ModeManagerConfig multipathConfig;
    multipathConfig.minHealthyCount = 1;
    multipathConfig.disagreementThreshold = 5.0;
    multipathConfig.maxDisagreementCount = 1;
    multipathConfig.lockoutSteps = 2;
    multipathConfig.maxResidualAgeSeconds = 0.1;
    multipathConfig.permittedSensors = {"lidar", "imu"};
    multipathConfig.ladderOrder = {"lio"};
    ModeManager multipathManager(multipathConfig);
    TestSensor multipathLidar("lidar");
    TestSensor multipathImu("imu");
    std::vector<SensorBase *> multipathSensors{&multipathLidar, &multipathImu};
    multipathLidar.setStatus(true, 0.1, 0.9);
    multipathImu.setStatus(true, 0.1, 0.9);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0}, 1.0);
    multipathImu.setPositionMeasurement({100.0, 0.0, 0.0}, 1.0);
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Hold);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0}, 1.2);
    multipathImu.setPositionMeasurement({0.0, 0.0, 0.0}, 1.5);
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Hold);

    multipathLidar.setPositionMeasurement({0.0, 0.0, 0.0}, 2.0);
    multipathImu.setPositionMeasurement({0.0, 0.0, 0.0}, 2.0);
    decision = multipathManager.decide(multipathSensors);
    assert(decision.mode == TrackingMode::Lio);

    ModeManagerConfig saturationConfig;
    saturationConfig.minHealthyCount = 1;
    saturationConfig.minConfidence = 0.5;
    saturationConfig.permittedSensors = {"thermal", "radar"};
    saturationConfig.ladderOrder = {"thermal", "radar"};
    ModeManager saturationManager(saturationConfig);
    TestSensor saturationThermal("thermal");
    TestSensor saturationRadar("radar");
    std::vector<SensorBase *> saturationSensors{&saturationThermal, &saturationRadar};
    saturationThermal.setStatus(true, 0.1, 0.1);
    saturationRadar.setStatus(true, 0.1, 0.9);
    decision = saturationManager.decide(saturationSensors);
    assert(decision.mode == TrackingMode::Radar);

    return 0;
}
