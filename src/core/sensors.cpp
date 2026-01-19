#include "core/sensors.h"

#include <cmath>

namespace
{
constexpr double kPi = 3.141592653589793;

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

double normalizeAngle(double angleRad)
{
    double wrapped = std::fmod(angleRad + kPi, 2.0 * kPi);
    if (wrapped < 0.0)
    {
        wrapped += 2.0 * kPi;
    }
    return wrapped - kPi;
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

VisionSensor::VisionSensor(const SensorConfig &config)
    : SensorBase("vision", config)
{
}

Measurement VisionSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
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
        measurement.position = Vec3{state.position.x + 20.0, state.position.y - 15.0, state.position.z + 5.0};
        measurement.valid = true;
        measurement.note = "false_positive";
        return measurement;
    }

    measurement.position = Vec3{
        state.position.x + gaussianNoise(config.noiseStd, rng),
        state.position.y + gaussianNoise(config.noiseStd, rng),
        state.position.z + gaussianNoise(config.noiseStd, rng)};
    measurement.valid = true;
    return measurement;
}

LidarSensor::LidarSensor(const SensorConfig &config)
    : SensorBase("lidar", config)
{
}

Measurement LidarSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
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

    if (randomEvent(config.falsePositiveProbability, rng))
    {
        measurement.range = range + 25.0;
        measurement.bearing = normalizeAngle(bearing + 0.15);
        measurement.valid = true;
        measurement.note = "spurious_reflection";
        return measurement;
    }

    measurement.range = range + gaussianNoise(config.noiseStd * 0.5, rng);
    measurement.bearing = normalizeAngle(bearing + gaussianNoise(config.noiseStd * 0.005, rng));
    measurement.valid = true;
    return measurement;
}

MagnetometerSensor::MagnetometerSensor(const SensorConfig &config)
    : SensorBase("magnetometer", config), bias(0.0)
{
}

Measurement MagnetometerSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    double speed = std::sqrt(state.velocity.x * state.velocity.x +
                             state.velocity.y * state.velocity.y +
                             state.velocity.z * state.velocity.z);
    if (speed < 0.01)
    {
        measurement.valid = false;
        measurement.note = "low_speed";
        return measurement;
    }

    bias += gaussianNoise(config.noiseStd * 0.01, rng);
    double heading = std::atan2(state.velocity.y, state.velocity.x);
    measurement.heading = normalizeAngle(heading + bias + gaussianNoise(config.noiseStd * 0.1, rng));
    measurement.valid = true;
    return measurement;
}

BarometerSensor::BarometerSensor(const SensorConfig &config)
    : SensorBase("baro", config), drift(0.0)
{
}

Measurement BarometerSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;
    if (config.maxRange > 0.0 && std::fabs(state.position.z) > config.maxRange)
    {
        measurement.valid = false;
        measurement.note = "out_of_range";
        return measurement;
    }

    drift += gaussianNoise(config.noiseStd * 0.05, rng);
    measurement.altitude = state.position.z + drift + gaussianNoise(config.noiseStd, rng);
    measurement.valid = true;
    return measurement;
}

CelestialSensor::CelestialSensor(const SensorConfig &config)
    : SensorBase("celestial", config), bias{0.0, 0.0, 0.0}
{
}

Measurement CelestialSensor::generateMeasurement(const State9 &state, std::mt19937 &rng)
{
    Measurement measurement;

    if (randomEvent(config.falsePositiveProbability, rng))
    {
        measurement.position = Vec3{state.position.x - 12.0, state.position.y + 18.0, state.position.z - 6.0};
        measurement.valid = true;
        measurement.note = "false_positive";
        return measurement;
    }

    bias.x += gaussianNoise(config.noiseStd * 0.02, rng);
    bias.y += gaussianNoise(config.noiseStd * 0.02, rng);
    bias.z += gaussianNoise(config.noiseStd * 0.02, rng);

    measurement.position = Vec3{
        state.position.x + bias.x + gaussianNoise(config.noiseStd, rng),
        state.position.y + bias.y + gaussianNoise(config.noiseStd, rng),
        state.position.z + bias.z + gaussianNoise(config.noiseStd, rng)};
    measurement.valid = true;
    return measurement;
}
