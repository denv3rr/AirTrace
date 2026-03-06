#include "ui/main.h"
#include "tools/audit_log.h"
#include "ui/simulation.h"

#include <cstdlib>

namespace
{
std::string resolveConfigPath(int argc, char **argv)
{
    if (argc > 1 && argv[1] != nullptr && argv[1][0] != '\0')
    {
        return argv[1];
    }
    const char *envConfigPath = std::getenv("AIRTRACE_CONFIG");
    if (envConfigPath != nullptr && envConfigPath[0] != '\0')
    {
        return envConfigPath;
    }
    return "configs/sim_default.cfg";
}
} // namespace

int main(int argc, char **argv)
{
    const std::string configPath = resolveConfigPath(argc, argv);
    tools::AuditLogConfig auditConfig{};
    auditConfig.logPath = "audit_log.jsonl";
    auditConfig.configPath = configPath;
    auditConfig.buildId = AIRTRACE_BUILD_ID;
    auditConfig.role = "unknown";
    std::string auditStatus;
    if (!tools::initializeAuditLog(auditConfig, auditStatus))
    {
        setUiDenialReason("audit_unavailable");
        std::cerr << "Error: audit logging unavailable (" << auditStatus << "). Exiting.\n";
        return 1;
    }
    if (!initializeUiContext(configPath))
    {
        tools::logAuditEvent("config_invalid", "configuration load failed", configPath);
        std::cerr << "Error: unable to load " << configPath << ". Exiting.\n";
        return 1;
    }
    tools::setAuditRole(getUiStatus().authStatus);
    setUiLoggingStatus(tools::auditLogStatus());
    tools::logAuditEvent("config_loaded", "configuration loaded", configPath);
    loadSimulationHistory();
    if (!showMainMenu())
    {
        saveSimulationHistory();
        return 1;
    }
    saveSimulationHistory();
    return 0;
}
