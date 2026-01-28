#include "ui/main.h"
#include "ui/audit_log.h"
#include "ui/simulation.h"

int main()
{
    const std::string configPath = "configs/sim_default.cfg";
    ui::AuditLogConfig auditConfig{};
    auditConfig.logPath = "audit_log.jsonl";
    auditConfig.configPath = configPath;
    auditConfig.buildId = AIRTRACE_BUILD_ID;
    auditConfig.role = "unknown";
    std::string auditStatus;
    if (!ui::initializeAuditLog(auditConfig, auditStatus))
    {
        setUiDenialReason("audit_unavailable");
        std::cerr << "Error: audit logging unavailable (" << auditStatus << "). Exiting.\n";
        return 1;
    }
    if (!initializeUiContext(configPath))
    {
        ui::logAuditEvent("config_invalid", "configuration load failed", configPath);
        std::cerr << "Error: unable to load configs/sim_default.cfg. Exiting.\n";
        return 1;
    }
    ui::setAuditRole(getUiStatus().authStatus);
    setUiLoggingStatus(ui::auditLogStatus());
    ui::logAuditEvent("config_loaded", "configuration loaded", configPath);
    loadSimulationHistory();
    if (!showMainMenu())
    {
        saveSimulationHistory();
        return 1;
    }
    saveSimulationHistory();
    return 0;
}
