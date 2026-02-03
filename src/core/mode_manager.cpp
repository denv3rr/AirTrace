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

bool isAllowedProvenance(const ModeManagerConfig &config, ProvenanceTag tag)
{
    if (config.allowedProvenances.empty())
    {
        return false;
    }
    for (const auto &allowed : config.allowedProvenances)
    {
        if (allowed == tag)
        {
            return true;
        }
    }
    return false;
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

bool modeAuthorized(const ModeManagerConfig &config, const std::string &modeName, std::string &reason)
{
    if (!config.authorizationRequired)
    {
        return true;
    }
    if (!config.authorizationVerified)
    {
        reason = "auth_unavailable";
        return false;
    }
    if (modeName == "hold")
    {
        return true;
    }
    for (const auto &allowed : config.authorizationAllowedModes)
    {
        if (allowed == modeName)
        {
            return true;
        }
    }
    reason = "auth_denied";
    return false;
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

void setLockout(std::unordered_map<std::string, int> &remaining,
                std::unordered_map<std::string, std::string> &reasons,
                const std::string &name,
                int steps,
                const std::string &reason)
{
    if (steps <= 0)
    {
        return;
    }
    remaining[name] = steps;
    if (reasons.find(name) == reasons.end())
    {
        reasons[name] = reason;
    }
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
                    setLockout(lockoutRemaining, lockoutReasons, name, config.lockoutSteps, "stale");
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
                    setLockout(lockoutRemaining, lockoutReasons, name, config.lockoutSteps, "low_confidence");
                }
            }
        }
    }

    auto eligibleSensor = [&](const std::string &name, std::string &reason) -> bool
    {
        if (config.allowedProvenances.empty())
        {
            reason = "provenance_unconfigured";
            return false;
        }
        if (name == "celestial" && !config.celestialDatasetAvailable)
        {
            reason = "dataset_unavailable";
            return false;
        }
        if (name == "celestial" && !config.celestialAllowed)
        {
            reason = "celestial_disallowed";
            return false;
        }
        if (!permitted(name))
        {
            reason = "not_permitted";
            return false;
        }
        auto lockoutIt = lockoutRemaining.find(name);
        if (lockoutIt != lockoutRemaining.end() && lockoutIt->second > 0)
        {
            reason = "lockout";
            return false;
        }
        auto statusIt = statusByName.find(name);
        if (statusIt == statusByName.end())
        {
            reason = "missing";
            return false;
        }
        const SensorStatus &status = statusIt->second;
        if (!status.hasMeasurement)
        {
            reason = "no_measurement";
            return false;
        }
        ProvenanceTag tag = status.lastMeasurement.provenance;
        if (tag == ProvenanceTag::Unknown)
        {
            reason = "provenance_unknown";
            return false;
        }
        if (!isAllowedProvenance(config, tag))
        {
            reason = "provenance_denied";
            return false;
        }
        if (status.timeSinceLastValid > config.maxDataAgeSeconds)
        {
            reason = "stale";
            return false;
        }
        if (status.confidence < config.minConfidence)
        {
            reason = "low_confidence";
            return false;
        }
        auto it = healthyCounts.find(name);
        if (it == healthyCounts.end())
        {
            reason = "unhealthy_count";
            return false;
        }
        if (it->second < config.minHealthyCount)
        {
            reason = "unhealthy_count";
            return false;
        }
        reason.clear();
        return true;
    };

    TrackingMode desiredMode = TrackingMode::Hold;
    std::string desiredReason = "no_modes";
    std::string denialReason;
    bool authDenied = false;
    bool invalidLadder = false;
    std::unordered_map<std::string, bool> conflictNow;
    const std::vector<std::string> &ladder = config.ladderOrder;
    std::vector<ModeDecisionDetail::DisqualifiedSource> disqualifiedSources;
    for (const auto &modeName : ladder)
    {
        if (modeName == "hold")
        {
            continue;
        }
        std::string authReason;
        if (!modeAuthorized(config, modeName, authReason))
        {
            disqualifiedSources.push_back({modeName, "authorization", authReason});
            denialReason = authReason;
            if (authReason == "auth_denied")
            {
                authDenied = true;
            }
            continue;
        }
        std::vector<std::string> required = requiredSensorsForMode(modeName);
        if (required.empty())
        {
            denialReason = "invalid_mode";
            disqualifiedSources.push_back({modeName, "mode", "invalid_mode"});
            invalidLadder = true;
            break;
        }
        bool eligible = true;
        for (const auto &sensorName : required)
        {
            std::string reason;
            if (!eligibleSensor(sensorName, reason))
            {
                disqualifiedSources.push_back({modeName, sensorName, reason});
                if (denialReason.empty() && reason.rfind("provenance_", 0) == 0)
                {
                    denialReason = reason;
                }
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
                for (const auto &sensorName : required)
                {
                    disqualifiedSources.push_back({modeName, sensorName, "residual_conflict"});
                }
                continue;
            }
            if (unaligned)
            {
                denialReason = "residual_unaligned";
                for (const auto &sensorName : required)
                {
                    disqualifiedSources.push_back({modeName, sensorName, "residual_unaligned"});
                }
                continue;
            }
        }
        if (!config.provenanceAllowMixed && required.size() > 1)
        {
            std::optional<ProvenanceTag> firstTag;
            bool mixed = false;
            for (const auto &sensorName : required)
            {
                auto it = statusByName.find(sensorName);
                if (it == statusByName.end())
                {
                    continue;
                }
                ProvenanceTag tag = it->second.lastMeasurement.provenance;
                if (!firstTag.has_value())
                {
                    firstTag = tag;
                }
                else if (firstTag.value() != tag)
                {
                    mixed = true;
                    break;
                }
            }
            if (mixed)
            {
                denialReason = "provenance_mixed";
                for (const auto &sensorName : required)
                {
                    disqualifiedSources.push_back({modeName, sensorName, "provenance_mixed"});
                }
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
                setLockout(lockoutRemaining, lockoutReasons, name, config.lockoutSteps, "disagreement");
            }
        }
    }

    auto finalizeDetail = [&](ModeDecisionDetail &detail)
    {
        detail.disqualifiedSources = disqualifiedSources;
        detail.lockouts.clear();
        for (const auto &entry : lockoutRemaining)
        {
            if (entry.second > 0)
            {
                ModeDecisionDetail::LockoutState lockout;
                lockout.source = entry.first;
                lockout.remainingSteps = entry.second;
                auto reasonIt = lockoutReasons.find(entry.first);
                if (reasonIt != lockoutReasons.end())
                {
                    lockout.reason = reasonIt->second;
                }
                detail.lockouts.push_back(lockout);
            }
        }
    };

    if (invalidLadder)
    {
        currentMode = TrackingMode::Hold;
        dwellSteps = 0;
        ModeDecision decision{TrackingMode::Hold, "invalid_ladder"};
        lastDecisionDetail = buildDecisionDetail("hold", decision.reason, sensors);
        lastDecisionDetail.downgradeReason = "invalid_ladder";
        finalizeDetail(lastDecisionDetail);
        return decision;
    }

    if (desiredMode == TrackingMode::Hold)
    {
        currentMode = TrackingMode::Hold;
        dwellSteps = 0;
        std::string finalReason = "no_sensors";
        if (config.authorizationRequired && !config.authorizationVerified)
        {
            finalReason = "auth_unavailable";
        }
        else if (authDenied)
        {
            finalReason = "auth_denied";
        }
        ModeDecision decision{TrackingMode::Hold, finalReason};
        lastDecisionDetail = buildDecisionDetail("hold", decision.reason, sensors);
        if (!denialReason.empty())
        {
            lastDecisionDetail.downgradeReason = denialReason;
        }
        finalizeDetail(lastDecisionDetail);
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
        finalizeDetail(lastDecisionDetail);
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
            std::string reason;
            if (!eligibleSensor(sensorName, reason))
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
        finalizeDetail(lastDecisionDetail);
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
        finalizeDetail(lastDecisionDetail);
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
        finalizeDetail(lastDecisionDetail);
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
    finalizeDetail(lastDecisionDetail);
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
