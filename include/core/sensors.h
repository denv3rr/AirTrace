#ifndef CORE_SENSORS_H
#define CORE_SENSORS_H

#include <optional>
#include <random>
#include <string>

#include "core/state.h"

struct Measurement
{
    std::optional<Vec3> position;
    std::optional<Vec3> velocity;
    std::optional<double> range;
    std::optional<double> bearing;
    bool valid = false;
    std::string note;
};

struct SensorStatus
{
    bool available = true;
    bool healthy = true;
    int missedUpdates = 0;
    std::string lastError;
};

struct SensorConfig
{
    double rateHz;
    double noiseStd;
    double dropoutProbability;
    double falsePositiveProbability;
    double maxRange;
};

class SensorBase
{
public:
    SensorBase(std::string name, SensorConfig config);
    virtual ~SensorBase() = default;

    Measurement sample(const State9 &state, double dt, std::mt19937 &rng);
    const std::string &getName() const;
    const SensorStatus &getStatus() const;

protected:
    virtual Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) = 0;
    void recordFailure(const std::string &reason);
    void recordSuccess();

    SensorConfig config;
    std::string name;
    SensorStatus status;
    double timeAccumulator;
};

class GpsSensor : public SensorBase
{
public:
    explicit GpsSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    Vec3 bias;
};

class ThermalSensor : public SensorBase
{
public:
    explicit ThermalSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;
};

class DeadReckoningSensor : public SensorBase
{
public:
    explicit DeadReckoningSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    Vec3 drift;
};

class ImuSensor : public SensorBase
{
public:
    explicit ImuSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    Vec3 bias;
};

class RadarSensor : public SensorBase
{
public:
    explicit RadarSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;
};

#endif // CORE_SENSORS_H
