#include "core/sensors.h"

#include <cmath>

namespace
{
double gaussianNoise(double stddev, std::mt19937 &rng)
{
    if (stddev <= 0.0)
    {
        return 0.0;
    }
    std::normal_distribution<double> noise(0.0, stddev);
    return noise(rng);
}

bool randomEvent(double probability, std::mt19937 &rng)
{
    if (probability <= 0.0)
    {
        return false;
    }
    std::bernoulli_distribution chance(probability);
    return chance(rng);
}
} // namespace

SensorBase::SensorBase(std::string name, SensorConfig config)
    : config(config), name(std::move(name)), timeAccumulator(0.0)
{
}

Measurement SensorBase::sample(const State9 &state, double dt, std::mt19937 &rng)
{
    Measurement measurement;
    timeAccumulator += dt;

    double period = (config.rateHz > 0.0) ? (1.0 / config.rateHz) : dt;
    if (timeAccumulator < period)
    {
        return measurement;
    }
    timeAccumulator = 0.0;

    if (!status.available || randomEvent(config.dropoutProbability, rng))
    {
        recordFailure("dropout");
        return measurement;
    }

    measurement = generateMeasurement(state, rng);
    if (!measurement.valid)
    {
        recordFailure(measurement.note);
        return measurement;
    }

    recordSuccess();
    return measurement;
}

const std::string &SensorBase::getName() const
{
    return name;
}

const SensorStatus &SensorBase::getStatus() const
{
    return status;
}

void SensorBase::recordFailure(const std::string &reason)
{
    status.missedUpdates += 1;
    status.healthy = false;
    status.lastError = reason;
}

void SensorBase::recordSuccess()
{
    status.missedUpdates = 0;
    status.healthy = true;
    status.lastError.clear();
}

GpsSensor::GpsSensor(const SensorConfig &config)
    : SensorBase("gps", config), bias{0.0, 0.0, 0.0}
{
}

Measurement GpsSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;

    if (randomEvent(config.falsePositiveProbability, rng))
    {
        measurement.position = Vec3{state.position.x + 100.0, state.position.y - 100.0, state.position.z + 50.0};
        measurement.valid = true;
        measurement.note = "false_positive";
        return measurement;
    }

    bias.x += gaussianNoise(config.noiseStd * 0.1, rng);
    bias.y += gaussianNoise(config.noiseStd * 0.1, rng);
    bias.z += gaussianNoise(config.noiseStd * 0.1, rng);

    measurement.position = Vec3{
        state.position.x + bias.x + gaussianNoise(config.noiseStd, rng),
        state.position.y + bias.y + gaussianNoise(config.noiseStd, rng),
        state.position.z + bias.z + gaussianNoise(config.noiseStd, rng)};
    measurement.valid = true;
    return measurement;
}

ThermalSensor::ThermalSensor(const SensorConfig &config)
    : SensorBase("thermal", config)
{
}

Measurement ThermalSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    double range = std::sqrt(state.position.x * state.position.x +
                             state.position.y * state.position.y +
                             state.position.z * state.position.z);

    if (range > config.maxRange)
    {
        measurement.valid = false;
        measurement.note = "out_of_range";
        return measurement;
    }

    if (randomEvent(config.falsePositiveProbability, rng))
    {
        measurement.position = Vec3{state.position.x + 30.0, state.position.y - 30.0, state.position.z + 10.0};
        measurement.valid = true;
        measurement.note = "flare";
        return measurement;
    }

    measurement.position = Vec3{
        state.position.x + gaussianNoise(config.noiseStd, rng),
        state.position.y + gaussianNoise(config.noiseStd, rng),
        state.position.z + gaussianNoise(config.noiseStd, rng)};
    measurement.valid = true;
    return measurement;
}

DeadReckoningSensor::DeadReckoningSensor(const SensorConfig &config)
    : SensorBase("dead_reckoning", config), drift{0.0, 0.0, 0.0}
{
}

Measurement DeadReckoningSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    drift.x += gaussianNoise(config.noiseStd * 0.2, rng);
    drift.y += gaussianNoise(config.noiseStd * 0.2, rng);
    drift.z += gaussianNoise(config.noiseStd * 0.2, rng);

    measurement.position = Vec3{
        state.position.x + drift.x,
        state.position.y + drift.y,
        state.position.z + drift.z};
    measurement.valid = true;
    return measurement;
}

ImuSensor::ImuSensor(const SensorConfig &config)
    : SensorBase("imu", config), bias{0.0, 0.0, 0.0}
{
}

Measurement ImuSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    bias.x += gaussianNoise(config.noiseStd * 0.05, rng);
    bias.y += gaussianNoise(config.noiseStd * 0.05, rng);
    bias.z += gaussianNoise(config.noiseStd * 0.05, rng);

    measurement.velocity = Vec3{
        state.velocity.x + bias.x + gaussianNoise(config.noiseStd, rng),
        state.velocity.y + bias.y + gaussianNoise(config.noiseStd, rng),
        state.velocity.z + bias.z + gaussianNoise(config.noiseStd, rng)};
    measurement.valid = true;
    return measurement;
}

RadarSensor::RadarSensor(const SensorConfig &config)
    : SensorBase("radar", config)
{
}

Measurement RadarSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    double range = std::sqrt(state.position.x * state.position.x +
                             state.position.y * state.position.y +
                             state.position.z * state.position.z);
    if (range > config.maxRange)
    {
        measurement.valid = false;
        measurement.note = "out_of_range";
        return measurement;
    }

    double bearing = std::atan2(state.position.y, state.position.x);
    measurement.range = range + gaussianNoise(config.noiseStd, rng);
    measurement.bearing = bearing + gaussianNoise(config.noiseStd * 0.01, rng);
    measurement.valid = true;
    return measurement;
}
