#include "core/mode_manager.h"
#include "core/sim_config.h"
#include "core/state.h"

#include <cassert>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <string>
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

    void setHealthy(bool healthy)
    {
        if (healthy)
        {
            recordSuccess();
        }
        else
        {
            recordFailure("forced");
        }
    }

protected:
    Measurement generateMeasurement(const State9 &, std::mt19937 &) override
    {
        Measurement measurement;
        measurement.valid = true;
        return measurement;
    }
};

std::filesystem::path writeConfigFile(const std::string &name, const std::string &contents)
{
    std::filesystem::path path = std::filesystem::current_path() / name;
    std::ofstream file(path.string(), std::ios::trunc);
    file << contents;
    return path;
}
} // namespace

int main()
{
    State9 state{};
    state.position = {0.0, 0.0, 0.0};
    state.velocity = {10.0, -5.0, 2.0};
    state.acceleration = {1.0, 0.0, -1.0};
    state.time = 0.0;

    const double dt = 2.0;
    State9 next = integrateState(state, dt);

    const double eps = 1e-9;
    assert(std::fabs(next.position.x - 22.0) < eps);
    assert(std::fabs(next.position.y - -10.0) < eps);
    assert(std::fabs(next.position.z - 2.0) < eps);
    assert(std::fabs(next.velocity.x - 12.0) < eps);
    assert(std::fabs(next.velocity.y - -5.0) < eps);
    assert(std::fabs(next.velocity.z - 0.0) < eps);
    assert(std::fabs(next.time - 2.0) < eps);

    Projection2D xy = projectXY(next);
    assert(xy.plane == "XY");
    assert(std::fabs(xy.x - 22.0) < eps);
    assert(std::fabs(xy.y - -10.0) < eps);

    std::filesystem::path missingVersion = writeConfigFile(
        "airtrace_missing_version.cfg",
        "sim.dt=0.2\n");
    ConfigResult missingResult = loadSimConfig(missingVersion.string());
    assert(!missingResult.ok);
    std::filesystem::remove(missingVersion);

    std::filesystem::path invalidDt = writeConfigFile(
        "airtrace_invalid_dt.cfg",
        "config.version=1.0\n"
        "sim.dt=0\n"
        "bounds.min.x=10\n"
        "bounds.max.x=0\n");
    ConfigResult invalidResult = loadSimConfig(invalidDt.string());
    assert(!invalidResult.ok);
    std::filesystem::remove(invalidDt);

    ModeManagerConfig modeConfig;
    modeConfig.minHealthyCount = 2;
    modeConfig.minDwellSteps = 2;

    ModeManager manager(modeConfig);
    TestSensor gps("gps");
    TestSensor thermal("thermal");
    std::vector<SensorBase *> sensors{&gps, &thermal};

    gps.setHealthy(true);
    thermal.setHealthy(false);
    ModeDecision decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Hold);

    gps.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    thermal.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    gps.setHealthy(false);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    gps.setHealthy(true);
    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Thermal);

    decision = manager.decide(sensors);
    assert(decision.mode == TrackingMode::Gps);

    return 0;
}
