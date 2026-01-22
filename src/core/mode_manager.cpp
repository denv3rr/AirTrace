#include "core/mode_manager.h"

#include <algorithm>
#include <cmath>
#include <optional>

namespace
{
double normalizeAngle(double radians)
{
    while (radians > 3.141592653589793)
    {
        radians -= 2.0 * 3.141592653589793;
    }
    while (radians < -3.141592653589793)
    {
        radians += 2.0 * 3.141592653589793;
    }
    return radians;
}

std::optional<double> residualBetween(const SensorStatus &a, const SensorStatus &b, double maxAgeSeconds)
{
    if (maxAgeSeconds > 0.0)
    {
        double ageDelta = std::fabs(a.lastMeasurementTime - b.lastMeasurementTime);
        if (ageDelta > maxAgeSeconds)
        {
            return std::nullopt;
        }
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.position && b.lastMeasurement.position)
    {
        Vec3 delta{
            a.lastMeasurement.position->x - b.lastMeasurement.position->x,
            a.lastMeasurement.position->y - b.lastMeasurement.position->y,
            a.lastMeasurement.position->z - b.lastMeasurement.position->z};
        return std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.velocity && b.lastMeasurement.velocity)
    {
        Vec3 delta{
            a.lastMeasurement.velocity->x - b.lastMeasurement.velocity->x,
            a.lastMeasurement.velocity->y - b.lastMeasurement.velocity->y,
            a.lastMeasurement.velocity->z - b.lastMeasurement.velocity->z};
        return std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.range && b.lastMeasurement.position)
    {
        double range = std::sqrt(b.lastMeasurement.position->x * b.lastMeasurement.position->x +
                                 b.lastMeasurement.position->y * b.lastMeasurement.position->y +
                                 b.lastMeasurement.position->z * b.lastMeasurement.position->z);
        return std::fabs(*a.lastMeasurement.range - range);
    }
    if (a.hasMeasurement && b.hasMeasurement && b.lastMeasurement.range && a.lastMeasurement.position)
    {
        double range = std::sqrt(a.lastMeasurement.position->x * a.lastMeasurement.position->x +
                                 a.lastMeasurement.position->y * a.lastMeasurement.position->y +
                                 a.lastMeasurement.position->z * a.lastMeasurement.position->z);
        return std::fabs(*b.lastMeasurement.range - range);
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.altitude && b.lastMeasurement.position)
    {
        return std::fabs(*a.lastMeasurement.altitude - b.lastMeasurement.position->z);
    }
    if (a.hasMeasurement && b.hasMeasurement && b.lastMeasurement.altitude && a.lastMeasurement.position)
    {
        return std::fabs(*b.lastMeasurement.altitude - a.lastMeasurement.position->z);
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.heading && b.lastMeasurement.heading)
    {
        return std::fabs(normalizeAngle(*a.lastMeasurement.heading - *b.lastMeasurement.heading));
    }
    if (a.hasMeasurement && b.hasMeasurement && a.lastMeasurement.bearing && b.lastMeasurement.bearing)
    {
        return std::fabs(normalizeAngle(*a.lastMeasurement.bearing - *b.lastMeasurement.bearing));
    }
    return std::nullopt;
}

bool alignedInTime(const SensorStatus &a, const SensorStatus &b, double maxAgeSeconds)
{
    if (maxAgeSeconds <= 0.0)
    {
        return true;
    }
    if (!a.hasMeasurement || !b.hasMeasurement)
    {
        return false;
    }
    double ageDelta = std::fabs(a.lastMeasurementTime - b.lastMeasurementTime);
    return ageDelta <= maxAgeSeconds;
}

void pushHistory(TrendHistory &history, int value, int window)
{
    if (window <= 0)
    {
        return;
    }
    std::size_t size = history.values.size();
    if (size < static_cast<std::size_t>(window))
    {
        history.values.push_back(value);
        history.sum += value;
        if (history.values.size() == static_cast<std::size_t>(window))
        {
            history.cursor = 0;
        }
        return;
    }
    history.sum -= history.values[history.cursor];
    history.values[history.cursor] = value;
    history.sum += value;
    history.cursor = (history.cursor + 1) % static_cast<std::size_t>(window);
}

std::vector<std::string> requiredSensorsForMode(const std::string &modeName)
{
    if (modeName == "gps_ins")
    {
        return {"gps", "imu"};
    }
    if (modeName == "vio")
    {
        return {"vision", "imu"};
    }
    if (modeName == "lio")
    {
        return {"lidar", "imu"};
    }
    if (modeName == "radar_inertial")
    {
        return {"radar", "imu"};
    }
    if (modeName == "mag_baro")
    {
        return {"magnetometer", "baro"};
    }
    if (modeName == "gps")
    {
        return {"gps"};
    }
    if (modeName == "vision")
    {
        return {"vision"};
    }
    if (modeName == "lidar")
    {
        return {"lidar"};
    }
    if (modeName == "radar")
    {
        return {"radar"};
    }
    if (modeName == "thermal")
    {
        return {"thermal"};
    }
    if (modeName == "magnetometer")
    {
        return {"magnetometer"};
    }
    if (modeName == "baro")
    {
        return {"baro"};
    }
    if (modeName == "celestial")
    {
        return {"celestial"};
    }
    if (modeName == "dead_reckoning")
    {
        return {"dead_reckoning"};
    }
    if (modeName == "imu")
    {
        return {"imu"};
    }
    return {};
}

TrackingMode modeFromName(const std::string &modeName)
{
    if (modeName == "gps_ins")
    {
        return TrackingMode::GpsIns;
    }
    if (modeName == "gps")
    {
        return TrackingMode::Gps;
    }
    if (modeName == "vio")
    {
        return TrackingMode::Vio;
    }
    if (modeName == "lio")
    {
        return TrackingMode::Lio;
    }
    if (modeName == "radar_inertial")
    {
        return TrackingMode::RadarInertial;
    }
    if (modeName == "thermal")
    {
        return TrackingMode::Thermal;
    }
    if (modeName == "radar")
    {
        return TrackingMode::Radar;
    }
    if (modeName == "vision")
    {
        return TrackingMode::Vision;
    }
    if (modeName == "lidar")
    {
        return TrackingMode::Lidar;
    }
    if (modeName == "mag_baro")
    {
        return TrackingMode::MagBaro;
    }
    if (modeName == "magnetometer")
    {
        return TrackingMode::Magnetometer;
    }
    if (modeName == "baro")
    {
        return TrackingMode::Baro;
    }
    if (modeName == "celestial")
    {
        return TrackingMode::Celestial;
    }
    if (modeName == "dead_reckoning")
    {
        return TrackingMode::DeadReckoning;
    }
    if (modeName == "imu")
    {
        return TrackingMode::Inertial;
    }
    return TrackingMode::Hold;
}

ModeDecisionDetail buildDecisionDetail(const std::string &modeName,
                                      const std::string &reason,
                                      const std::vector<SensorBase *> &sensors)
{
    ModeDecisionDetail detail;
    detail.selectedMode = modeName;
    detail.reason = reason;
    detail.contributors = requiredSensorsForMode(modeName);
    double confidence = 1.0;
    for (const auto &sensorName : detail.contributors)
    {
        double bestConfidence = 0.0;
        for (auto *sensor : sensors)
        {
            if (sensor && sensor->getName() == sensorName)
            {
                bestConfidence = std::max(bestConfidence, sensor->getStatus().confidence);
            }
        }
        confidence = std::min(confidence, bestConfidence);
    }
    if (detail.contributors.empty())
    {
        confidence = 0.0;
    }
    detail.confidence = confidence;
    return detail;
}
} // namespace

ModeManager::ModeManager(ModeManagerConfig config)
    : config(config)
{
}

ModeDecision ModeManager::decide(const std::vector<SensorBase *> &sensors)
{
    for (auto &entry : lockoutRemaining)
    {
        if (entry.second > 0)
        {
            entry.second -= 1;
        }
    }

    auto permitted = [&](const std::string &name) -> bool
    {
        if (name == "celestial" && !config.celestialAllowed)
        {
            return false;
        }
        if (name == "celestial" && !config.celestialDatasetAvailable)
        {
            return false;
        }
        if (config.permittedSensors.empty())
        {
            return true;
        }
        for (const auto &allowed : config.permittedSensors)
        {
            if (allowed == name)
            {
                return true;
            }
        }
        return false;
    };

    std::unordered_map<std::string, bool> healthyNow;
    std::unordered_map<std::string, SensorStatus> statusByName;
    for (auto *sensor : sensors)
    {
        if (!sensor)
        {
            continue;
        }
        const std::string &name = sensor->getName();
        bool &seenHealthy = healthyNow[name];
        const SensorStatus &status = sensor->getStatus();
        seenHealthy = seenHealthy || status.healthy;
        auto it = statusByName.find(name);
        if (it == statusByName.end())
        {
            statusByName[name] = status;
        }
        else
        {
            SensorStatus &current = it->second;
            if (status.timeSinceLastValid < current.timeSinceLastValid)
            {
                current = status;
            }
            else if (status.timeSinceLastValid == current.timeSinceLastValid &&
                     status.confidence > current.confidence)
            {
                current = status;
            }
        }

        int staleFlag = (status.timeSinceLastValid > config.maxDataAgeSeconds) ? 1 : 0;
        if (config.historyWindow > 0)
        {
            pushHistory(staleHistory[name], staleFlag, config.historyWindow);
        }
        else if (config.maxStaleCount > 0)
        {
            int &count = staleCounts[name];
            if (staleFlag == 1)
            {
                count += 1;
            }
            else
            {
                count = 0;
            }
        }

        int lowConfidenceFlag = (status.confidence < config.minConfidence) ? 1 : 0;
        if (config.historyWindow > 0)
        {
            pushHistory(lowConfidenceHistory[name], lowConfidenceFlag, config.historyWindow);
        }
        else if (config.maxLowConfidenceCount > 0)
        {
            int &count = lowConfidenceCounts[name];
            if (lowConfidenceFlag == 1)
            {
                count += 1;
            }
            else
            {
                count = 0;
            }
        }
    }

    for (const auto &entry : healthyNow)
    {
        int &count = healthyCounts[entry.first];
        count = entry.second ? (count + 1) : 0;
    }
    for (auto &entry : healthyCounts)
    {
        if (healthyNow.find(entry.first) == healthyNow.end())
        {
            entry.second = 0;
        }
    }

    if (config.lockoutSteps > 0)
    {
        for (const auto &entry : statusByName)
        {
            const std::string &name = entry.first;
            if (config.maxStaleCount > 0)
            {
                int staleCount = 0;
                if (config.historyWindow > 0)
                {
                    staleCount = staleHistory[name].sum;
                }
                else
                {
                    staleCount = staleCounts[name];
                }
                if (staleCount >= config.maxStaleCount)
                {
                    lockoutRemaining[name] = config.lockoutSteps;
                }
            }
            if (config.maxLowConfidenceCount > 0)
            {
                int lowCount = 0;
                if (config.historyWindow > 0)
                {
                    lowCount = lowConfidenceHistory[name].sum;
                }
                else
                {
                    lowCount = lowConfidenceCounts[name];
                }
                if (lowCount >= config.maxLowConfidenceCount)
                {
                    lockoutRemaining[name] = config.lockoutSteps;
                }
            }
        }
    }

    auto eligibleSensor = [&](const std::string &name) -> bool
    {
        if (!permitted(name))
        {
            return false;
        }
        auto lockoutIt = lockoutRemaining.find(name);
        if (lockoutIt != lockoutRemaining.end() && lockoutIt->second > 0)
        {
            return false;
        }
        auto statusIt = statusByName.find(name);
        if (statusIt == statusByName.end())
        {
            return false;
        }
        const SensorStatus &status = statusIt->second;
        if (status.timeSinceLastValid > config.maxDataAgeSeconds)
        {
            return false;
        }
        if (status.confidence < config.minConfidence)
        {
            return false;
        }
        auto it = healthyCounts.find(name);
        if (it == healthyCounts.end())
        {
            return false;
        }
        return it->second >= config.minHealthyCount;
    };

    TrackingMode desiredMode = TrackingMode::Hold;
    std::string desiredReason = "no_modes";
    std::string denialReason;
    std::unordered_map<std::string, bool> conflictNow;
    const std::vector<std::string> &ladder = config.ladderOrder;
    for (const auto &modeName : ladder)
    {
        if (modeName == "hold")
        {
            continue;
        }
        std::vector<std::string> required = requiredSensorsForMode(modeName);
        if (required.empty())
        {
            continue;
        }
        bool eligible = true;
        for (const auto &sensorName : required)
        {
            if (!eligibleSensor(sensorName))
            {
                eligible = false;
                break;
            }
        }
        if (!eligible)
        {
            continue;
        }
        if (config.disagreementThreshold > 0.0 && required.size() > 1)
        {
            bool conflict = false;
            bool unaligned = false;
            for (size_t i = 0; i < required.size(); ++i)
            {
                auto itA = statusByName.find(required[i]);
                if (itA == statusByName.end())
                {
                    continue;
                }
                for (size_t j = i + 1; j < required.size(); ++j)
                {
                    auto itB = statusByName.find(required[j]);
                    if (itB == statusByName.end())
                    {
                        continue;
                    }
                    if (!alignedInTime(itA->second, itB->second, config.maxResidualAgeSeconds))
                    {
                        unaligned = true;
                        continue;
                    }
                    std::optional<double> residual = residualBetween(itA->second, itB->second, config.maxResidualAgeSeconds);
                    if (residual && *residual > config.disagreementThreshold)
                    {
                        conflict = true;
                        conflictNow[required[i]] = true;
                        conflictNow[required[j]] = true;
                    }
                }
            }
            if (conflict)
            {
                denialReason = "residual_conflict";
                continue;
            }
            if (unaligned)
            {
                denialReason = "residual_unaligned";
                continue;
            }
        }
        desiredMode = modeFromName(modeName);
        desiredReason = modeName + "_eligible";
        break;
    }

    if (config.maxDisagreementCount > 0)
    {
        for (const auto &entry : statusByName)
        {
            const std::string &name = entry.first;
            int conflictFlag = conflictNow[name] ? 1 : 0;
            if (config.historyWindow > 0)
            {
                pushHistory(disagreementHistory[name], conflictFlag, config.historyWindow);
            }
            else
            {
                int &count = disagreementCounts[name];
                if (conflictFlag == 1)
                {
                    count += 1;
                }
                else
                {
                    count = 0;
                }
            }
            int countValue = 0;
            if (config.historyWindow > 0)
            {
                countValue = disagreementHistory[name].sum;
            }
            else
            {
                countValue = disagreementCounts[name];
            }
            if (config.lockoutSteps > 0 && countValue >= config.maxDisagreementCount)
            {
                lockoutRemaining[name] = config.lockoutSteps;
            }
        }
    }

    if (desiredMode == TrackingMode::Hold)
    {
        currentMode = TrackingMode::Hold;
        dwellSteps = 0;
        ModeDecision decision{TrackingMode::Hold, "no_sensors"};
        lastDecisionDetail = buildDecisionDetail("hold", decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        return decision;
    }

    if (currentMode == TrackingMode::Hold)
    {
        currentMode = desiredMode;
        dwellSteps = 0;
        ModeDecision decision{currentMode, "enter_" + modeName(currentMode)};
        lastDecisionDetail = buildDecisionDetail(modeName(currentMode), decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        return decision;
    }

    std::string currentModeName = modeName(currentMode);
    bool currentEligible = false;
    std::vector<std::string> required = requiredSensorsForMode(currentModeName);
    if (!required.empty())
    {
        currentEligible = true;
        for (const auto &sensorName : required)
        {
            if (!eligibleSensor(sensorName))
            {
                currentEligible = false;
                break;
            }
        }
    }

    if (currentMode == desiredMode)
    {
        dwellSteps += 1;
        ModeDecision decision{currentMode, "maintain_" + modeName(currentMode)};
        lastDecisionDetail = buildDecisionDetail(modeName(currentMode), decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        return decision;
    }

    if (!currentEligible)
    {
        currentMode = desiredMode;
        dwellSteps = 0;
        ModeDecision decision{currentMode, "switch_unhealthy_" + modeName(currentMode)};
        lastDecisionDetail = buildDecisionDetail(modeName(currentMode), decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        return decision;
    }

    if (dwellSteps < config.minDwellSteps)
    {
        dwellSteps += 1;
        ModeDecision decision{currentMode, "dwell_" + modeName(currentMode)};
        lastDecisionDetail = buildDecisionDetail(modeName(currentMode), decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        return decision;
    }

    currentMode = desiredMode;
    dwellSteps = 0;
    ModeDecision decision{currentMode, "switch_" + modeName(currentMode)};
    lastDecisionDetail = buildDecisionDetail(modeName(currentMode), decision.reason, sensors);
    if (!denialReason.empty())
    {
        lastDecisionDetail.downgradeReason = denialReason;
    }
    return decision;
}

ModeDecisionDetail ModeManager::decideDetailed(const std::vector<SensorBase *> &sensors)
{
    decide(sensors);
    return lastDecisionDetail;
}

const ModeDecisionDetail &ModeManager::getLastDecisionDetail() const
{
    return lastDecisionDetail;
}

std::string ModeManager::modeName(TrackingMode mode)
{
    switch (mode)
    {
    case TrackingMode::GpsIns:
        return "gps_ins";
    case TrackingMode::Gps:
        return "gps";
    case TrackingMode::Vio:
        return "vio";
    case TrackingMode::Lio:
        return "lio";
    case TrackingMode::RadarInertial:
        return "radar_inertial";
    case TrackingMode::Thermal:
        return "thermal";
    case TrackingMode::Radar:
        return "radar";
    case TrackingMode::Vision:
        return "vision";
    case TrackingMode::Lidar:
        return "lidar";
    case TrackingMode::MagBaro:
        return "mag_baro";
    case TrackingMode::Magnetometer:
        return "magnetometer";
    case TrackingMode::Baro:
        return "baro";
    case TrackingMode::Celestial:
        return "celestial";
    case TrackingMode::DeadReckoning:
        return "dead_reckoning";
    case TrackingMode::Inertial:
        return "inertial";
    case TrackingMode::Hold:
        return "hold";
    default:
        return "unknown";
    }
}
