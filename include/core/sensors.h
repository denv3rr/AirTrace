#ifndef CORE_SENSORS_H
#define CORE_SENSORS_H

#include <optional>
#include <random>
#include <string>

#include "core/state.h"

struct Measurement
{
    // Units: position/altitude in meters, velocity in m/s, range in meters, bearing/heading in radians.
    std::optional<Vec3> position;
    std::optional<Vec3> velocity;
    std::optional<double> range;
    std::optional<double> bearing;
    std::optional<double> altitude;
    std::optional<double> heading;
    bool valid = false;
    std::string note;
};

struct SensorStatus
{
    bool available = true;
    bool healthy = true;
    int missedUpdates = 0;
    double timeSinceLastValid = 0.0;
    double confidence = 1.0;
    bool hasMeasurement = false;
    Measurement lastMeasurement{};
    double lastMeasurementTime = 0.0;
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

class VisionSensor : public SensorBase
{
public:
    explicit VisionSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;
};

class LidarSensor : public SensorBase
{
public:
    explicit LidarSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;
};

class MagnetometerSensor : public SensorBase
{
public:
    explicit MagnetometerSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    double bias;
};

class BarometerSensor : public SensorBase
{
public:
    explicit BarometerSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    double drift;
};

class CelestialSensor : public SensorBase
{
public:
    explicit CelestialSensor(const SensorConfig &config);

protected:
    Measurement generateMeasurement(const State9 &state, std::mt19937 &rng) override;

private:
    Vec3 bias;
};

#endif // CORE_SENSORS_H
