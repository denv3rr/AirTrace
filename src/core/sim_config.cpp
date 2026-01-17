#include "core/sim_config.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace
{
constexpr const char *kExpectedVersion = "1.0";

std::string trim(const std::string &value)
{
    size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos)
    {
        return "";
    }
    size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

bool toDouble(const std::string &value, double &out)
{
    char *end = nullptr;
    out = std::strtod(value.c_str(), &end);
    return end && *end == '\0';
}

bool toInt(const std::string &value, int &out)
{
    char *end = nullptr;
    long temp = std::strtol(value.c_str(), &end, 10);
    if (!end || *end != '\0')
    {
        return false;
    }
    out = static_cast<int>(temp);
    return true;
}

bool toUnsigned(const std::string &value, unsigned int &out)
{
    char *end = nullptr;
    unsigned long temp = std::strtoul(value.c_str(), &end, 10);
    if (!end || *end != '\0')
    {
        return false;
    }
    out = static_cast<unsigned int>(temp);
    return true;
}

void setIssue(ConfigResult &result, const std::string &key, const std::string &message)
{
    result.ok = false;
    result.issues.push_back({key, message});
}

void applyValue(SimConfig &config, ConfigResult &result, const std::string &key, const std::string &value)
{
    double dval = 0.0;
    int ival = 0;
    unsigned int uval = 0;

    if (key == "config.version")
    {
        config.version = value;
    }
    else if (key == "state.position.x" && toDouble(value, dval)) config.initialState.position.x = dval;
    else if (key == "state.position.y" && toDouble(value, dval)) config.initialState.position.y = dval;
    else if (key == "state.position.z" && toDouble(value, dval)) config.initialState.position.z = dval;
    else if (key == "state.velocity.x" && toDouble(value, dval)) config.initialState.velocity.x = dval;
    else if (key == "state.velocity.y" && toDouble(value, dval)) config.initialState.velocity.y = dval;
    else if (key == "state.velocity.z" && toDouble(value, dval)) config.initialState.velocity.z = dval;
    else if (key == "state.acceleration.x" && toDouble(value, dval)) config.initialState.acceleration.x = dval;
    else if (key == "state.acceleration.y" && toDouble(value, dval)) config.initialState.acceleration.y = dval;
    else if (key == "state.acceleration.z" && toDouble(value, dval)) config.initialState.acceleration.z = dval;
    else if (key == "state.time" && toDouble(value, dval)) config.initialState.time = dval;
    else if (key == "sim.dt" && toDouble(value, dval)) config.dt = dval;
    else if (key == "sim.steps" && toInt(value, ival)) config.steps = ival;
    else if (key == "sim.seed" && toUnsigned(value, uval)) config.seed = uval;
    else if (key == "bounds.min.x" && toDouble(value, dval)) config.bounds.minPosition.x = dval;
    else if (key == "bounds.min.y" && toDouble(value, dval)) config.bounds.minPosition.y = dval;
    else if (key == "bounds.min.z" && toDouble(value, dval)) config.bounds.minPosition.z = dval;
    else if (key == "bounds.max.x" && toDouble(value, dval)) config.bounds.maxPosition.x = dval;
    else if (key == "bounds.max.y" && toDouble(value, dval)) config.bounds.maxPosition.y = dval;
    else if (key == "bounds.max.z" && toDouble(value, dval)) config.bounds.maxPosition.z = dval;
    else if (key == "bounds.max_speed" && toDouble(value, dval)) config.bounds.maxSpeed = dval;
    else if (key == "bounds.max_accel" && toDouble(value, dval)) config.bounds.maxAcceleration = dval;
    else if (key == "bounds.max_turn_rate_deg" && toDouble(value, dval)) config.bounds.maxTurnRateDeg = dval;
    else if (key == "maneuver.random_accel_std" && toDouble(value, dval)) config.maneuvers.randomAccelStd = dval;
    else if (key == "maneuver.probability" && toDouble(value, dval)) config.maneuvers.maneuverProbability = dval;
    else if (key == "sensor.gps.rate_hz" && toDouble(value, dval)) config.gps.rateHz = dval;
    else if (key == "sensor.gps.noise_std" && toDouble(value, dval)) config.gps.noiseStd = dval;
    else if (key == "sensor.gps.dropout" && toDouble(value, dval)) config.gps.dropoutProbability = dval;
    else if (key == "sensor.gps.false_positive" && toDouble(value, dval)) config.gps.falsePositiveProbability = dval;
    else if (key == "sensor.gps.max_range" && toDouble(value, dval)) config.gps.maxRange = dval;
    else if (key == "sensor.thermal.rate_hz" && toDouble(value, dval)) config.thermal.rateHz = dval;
    else if (key == "sensor.thermal.noise_std" && toDouble(value, dval)) config.thermal.noiseStd = dval;
    else if (key == "sensor.thermal.dropout" && toDouble(value, dval)) config.thermal.dropoutProbability = dval;
    else if (key == "sensor.thermal.false_positive" && toDouble(value, dval)) config.thermal.falsePositiveProbability = dval;
    else if (key == "sensor.thermal.max_range" && toDouble(value, dval)) config.thermal.maxRange = dval;
    else if (key == "sensor.dead_reckoning.rate_hz" && toDouble(value, dval)) config.deadReckoning.rateHz = dval;
    else if (key == "sensor.dead_reckoning.noise_std" && toDouble(value, dval)) config.deadReckoning.noiseStd = dval;
    else if (key == "sensor.dead_reckoning.dropout" && toDouble(value, dval)) config.deadReckoning.dropoutProbability = dval;
    else if (key == "sensor.dead_reckoning.false_positive" && toDouble(value, dval)) config.deadReckoning.falsePositiveProbability = dval;
    else if (key == "sensor.dead_reckoning.max_range" && toDouble(value, dval)) config.deadReckoning.maxRange = dval;
    else if (key == "sensor.imu.rate_hz" && toDouble(value, dval)) config.imu.rateHz = dval;
    else if (key == "sensor.imu.noise_std" && toDouble(value, dval)) config.imu.noiseStd = dval;
    else if (key == "sensor.imu.dropout" && toDouble(value, dval)) config.imu.dropoutProbability = dval;
    else if (key == "sensor.imu.false_positive" && toDouble(value, dval)) config.imu.falsePositiveProbability = dval;
    else if (key == "sensor.imu.max_range" && toDouble(value, dval)) config.imu.maxRange = dval;
    else if (key == "sensor.radar.rate_hz" && toDouble(value, dval)) config.radar.rateHz = dval;
    else if (key == "sensor.radar.noise_std" && toDouble(value, dval)) config.radar.noiseStd = dval;
    else if (key == "sensor.radar.dropout" && toDouble(value, dval)) config.radar.dropoutProbability = dval;
    else if (key == "sensor.radar.false_positive" && toDouble(value, dval)) config.radar.falsePositiveProbability = dval;
    else if (key == "sensor.radar.max_range" && toDouble(value, dval)) config.radar.maxRange = dval;
    else
    {
        setIssue(result, key, "unknown or invalid value");
    }
}

void validateRange(ConfigResult &result, const std::string &key, double value, double minValue, double maxValue,
                   bool minInclusive = true, bool maxInclusive = true)
{
    bool below = minInclusive ? (value < minValue) : (value <= minValue);
    bool above = maxInclusive ? (value > maxValue) : (value >= maxValue);
    if (below || above)
    {
        setIssue(result, key, "out of range");
    }
}

void validateConfig(ConfigResult &result)
{
    const SimConfig &config = result.config;

    if (config.version != kExpectedVersion)
    {
        setIssue(result, "config.version", "unsupported version");
    }

    validateRange(result, "state.position.x", config.initialState.position.x, -1e6, 1e6);
    validateRange(result, "state.position.y", config.initialState.position.y, -1e6, 1e6);
    validateRange(result, "state.position.z", config.initialState.position.z, -1e6, 1e6);
    validateRange(result, "state.velocity.x", config.initialState.velocity.x, -1e4, 1e4);
    validateRange(result, "state.velocity.y", config.initialState.velocity.y, -1e4, 1e4);
    validateRange(result, "state.velocity.z", config.initialState.velocity.z, -1e4, 1e4);
    validateRange(result, "state.acceleration.x", config.initialState.acceleration.x, -1e3, 1e3);
    validateRange(result, "state.acceleration.y", config.initialState.acceleration.y, -1e3, 1e3);
    validateRange(result, "state.acceleration.z", config.initialState.acceleration.z, -1e3, 1e3);
    validateRange(result, "state.time", config.initialState.time, 0.0, 1e6);

    validateRange(result, "sim.dt", config.dt, 0.0, 10.0, false, true);
    validateRange(result, "sim.steps", static_cast<double>(config.steps), 1.0, 1e7);
    validateRange(result, "sim.seed", static_cast<double>(config.seed), 0.0, 4294967295.0);

    validateRange(result, "bounds.min.x", config.bounds.minPosition.x, -1e6, 1e6);
    validateRange(result, "bounds.min.y", config.bounds.minPosition.y, -1e6, 1e6);
    validateRange(result, "bounds.min.z", config.bounds.minPosition.z, -1e6, 1e6);
    validateRange(result, "bounds.max.x", config.bounds.maxPosition.x, -1e6, 1e6);
    validateRange(result, "bounds.max.y", config.bounds.maxPosition.y, -1e6, 1e6);
    validateRange(result, "bounds.max.z", config.bounds.maxPosition.z, -1e6, 1e6);
    validateRange(result, "bounds.max_speed", config.bounds.maxSpeed, 0.0, 1e5, false, true);
    validateRange(result, "bounds.max_accel", config.bounds.maxAcceleration, 0.0, 1e4, false, true);
    validateRange(result, "bounds.max_turn_rate_deg", config.bounds.maxTurnRateDeg, 0.0, 360.0, false, true);

    if (config.bounds.minPosition.x > config.bounds.maxPosition.x ||
        config.bounds.minPosition.y > config.bounds.maxPosition.y ||
        config.bounds.minPosition.z > config.bounds.maxPosition.z)
    {
        setIssue(result, "bounds", "min greater than max");
    }

    validateRange(result, "maneuver.random_accel_std", config.maneuvers.randomAccelStd, 0.0, 1e3);
    validateRange(result, "maneuver.probability", config.maneuvers.maneuverProbability, 0.0, 1.0);

    auto validateSensor = [&](const SensorConfig &sensor, const std::string &name, bool rangeRequired)
    {
        validateRange(result, "sensor." + name + ".rate_hz", sensor.rateHz, 0.0, 2000.0, false, true);
        validateRange(result, "sensor." + name + ".noise_std", sensor.noiseStd, 0.0, 1e4);
        validateRange(result, "sensor." + name + ".dropout", sensor.dropoutProbability, 0.0, 1.0);
        validateRange(result, "sensor." + name + ".false_positive", sensor.falsePositiveProbability, 0.0, 1.0);
        if (rangeRequired)
        {
            validateRange(result, "sensor." + name + ".max_range", sensor.maxRange, 0.0, 1e7, false, true);
        }
    };

    validateSensor(config.gps, "gps", true);
    validateSensor(config.thermal, "thermal", true);
    validateSensor(config.deadReckoning, "dead_reckoning", true);
    validateSensor(config.imu, "imu", true);
    validateSensor(config.radar, "radar", true);
}
} // namespace

ConfigResult loadSimConfig(const std::string &path)
{
    ConfigResult result;
    result.config.initialState = {{0.0, 0.0, 100.0}, {15.0, 10.0, 0.0}, {0.2, -0.1, 0.0}, 0.0};

    std::ifstream file(path);
    if (!file)
    {
        setIssue(result, path, "unable to open config");
        return result;
    }

    std::string line;
    bool versionSeen = false;
    while (std::getline(file, line))
    {
        std::string trimmed = trim(line);
        if (trimmed.empty() || trimmed[0] == '#')
        {
            continue;
        }

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos)
        {
            setIssue(result, trimmed, "missing '='");
            continue;
        }

        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));
        if (key == "config.version")
        {
            versionSeen = true;
        }
        applyValue(result.config, result, key, value);
    }

    if (!versionSeen)
    {
        setIssue(result, "config.version", "missing required key");
    }
    validateConfig(result);

    return result;
}
