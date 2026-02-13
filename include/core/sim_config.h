#ifndef CORE_SIM_CONFIG_H
#define CORE_SIM_CONFIG_H

#include <string>
#include <unordered_map>
#include <vector>

#include "core/multi_modal_types.h"
#include "core/motion_models.h"
#include "core/sensors.h"
#include "core/state.h"

struct SimConfig
{
    std::string version = "1.0";
    State9 initialState{};
    double dt = 0.2;
    int steps = 20;
    unsigned int seed = 42;

    enum class PlatformProfile
    {
        Base,
        Air,
        Ground,
        Maritime,
        Space,
        Handheld,
        FixedSite,
        Subsea
    };

    enum class NetworkAidMode
    {
        Deny,
        Allow,
        TestOnly
    };

    enum class OverrideAuth
    {
        Credential,
        Key,
        Token
    };

    enum class DatasetTier
    {
        Minimal,
        Standard,
        Extended
    };

    enum class ProvenanceMode
    {
        Operational,
        Simulation,
        Test
    };

    enum class UnknownProvenanceAction
    {
        Deny,
        Hold
    };

    struct PolicyConfig
    {
        struct AuthorizationBundle
        {
            std::string version{};
            std::string source{};
            std::vector<std::string> allowedModes{};

            bool configured() const
            {
                return !version.empty() || !source.empty() || !allowedModes.empty();
            }
        };

        NetworkAidMode networkAidMode = NetworkAidMode::Deny;
        bool overrideRequired = true;
        OverrideAuth overrideAuth = OverrideAuth::Credential;
        int overrideTimeoutSeconds = 0;
        std::vector<std::string> roles = {"operator"};
        std::unordered_map<std::string, std::vector<std::string>> rolePermissions{};
        std::string activeRole = "operator";
        AuthorizationBundle authorization{};
    };

    struct ProvenancePolicy
    {
        ProvenanceMode runMode = ProvenanceMode::Operational;
        std::vector<ProvenanceMode> allowedInputs = {ProvenanceMode::Operational};
        bool allowMixed = false;
        UnknownProvenanceAction unknownAction = UnknownProvenanceAction::Deny;
    };

    struct DatasetConfig
    {
        DatasetTier tier = DatasetTier::Minimal;
        double maxSizeMB = 0.0;
        std::string celestialCatalogPath;
        std::string celestialEphemerisPath;
        std::string celestialCatalogHash;
        std::string celestialEphemerisHash;

        bool celestialAvailable() const
        {
            return !celestialCatalogPath.empty() && !celestialEphemerisPath.empty() &&
                   !celestialCatalogHash.empty() && !celestialEphemerisHash.empty();
        }
    };

    struct ModeConfig
    {
        std::vector<std::string> ladderOrder{};
        int minHealthyCount = 2;
        int minDwellSteps = 3;
        int maxStaleCount = 0;
        int maxLowConfidenceCount = 0;
        int lockoutSteps = 0;
        int historyWindow = 0;
    };

    struct FusionConfig
    {
        double maxDataAgeSeconds = 1.0;
        double disagreementThreshold = 50.0;
        double minConfidence = 0.0;
        int maxDisagreementCount = 0;
        double maxResidualAgeSeconds = 0.5;
        std::unordered_map<std::string, double> sourceWeights{};
    };

    struct AdapterConfig
    {
        std::string id;
        std::string version;
        std::string manifestPath;
        std::string allowlistPath;
        std::string uiSurface = "tui";
        std::string coreVersion = "1.0.0";
        std::string toolsVersion = "1.0.0";
        std::string uiVersion = "1.0.0";
        std::string adapterContractVersion = "1.0.0";
        std::string uiContractVersion = "1.0.0";
        int allowlistMaxAgeDays = 365;
    };

    struct PluginConfig
    {
        std::string id{};
        std::string version{};
        std::string signatureHash{};
        std::string signatureAlgorithm{};
        std::string allowlistId{};
        std::string allowlistVersion{};
        std::string allowlistSignatureHash{};
        std::string allowlistSignatureAlgorithm{};
        bool authorizationRequired = true;
        bool authorizationGranted = false;
        bool deviceDriver = false;
    };

    PlatformProfile platformProfile = PlatformProfile::Base;
    bool hasParentProfile = false;
    PlatformProfile parentProfile = PlatformProfile::Base;
    std::vector<std::string> childModules{};
    std::vector<std::string> permittedSensors{};
    PolicyConfig policy{};
    ProvenancePolicy provenance{};
    DatasetConfig dataset{};
    AdapterConfig adapter{};
    PluginConfig plugin{};
    ModeConfig mode{};
    FusionConfig fusion{};
    SchedulerConfig scheduler{};

    MotionBounds bounds{{-1000.0, -1000.0, 0.0}, {1000.0, 1000.0, 1000.0}, 250.0, 20.0, 12.0};
    ManeuverParams maneuvers{3.0, 0.35};

    SensorConfig gps{1.0, 2.0, 0.1, 0.03, 5000.0};
    SensorConfig thermal{5.0, 5.0, 0.15, 0.08, 1200.0};
    SensorConfig deadReckoning{20.0, 0.5, 0.0, 0.0, 1e6};
    SensorConfig imu{50.0, 0.2, 0.05, 0.0, 1e6};
    SensorConfig radar{2.0, 1.0, 0.08, 0.02, 2000.0};
    SensorConfig vision{10.0, 1.5, 0.1, 0.03, 1500.0};
    SensorConfig lidar{5.0, 0.5, 0.05, 0.02, 1000.0};
    SensorConfig magnetometer{20.0, 0.05, 0.02, 0.0, 1e6};
    SensorConfig baro{2.0, 0.3, 0.02, 0.0, 20000.0};
    SensorConfig celestial{0.2, 0.8, 0.15, 0.01, 1e7};
};

struct ConfigIssue
{
    std::string key;
    std::string message;
};

struct ConfigResult
{
    SimConfig config;
    std::vector<ConfigIssue> issues;
    bool ok = true;
};

#endif // CORE_SIM_CONFIG_H
