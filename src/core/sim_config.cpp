#include "core/sim_config.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

namespace
{
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

    if (key == "state.position.x" && toDouble(value, dval)) config.initialState.position.x = dval;
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
        applyValue(result.config, result, key, value);
    }

    return result;
}
