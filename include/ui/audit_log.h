#ifndef UI_AUDIT_LOG_H
#define UI_AUDIT_LOG_H

#include <string>

namespace ui
{
struct AuditLogConfig
{
    std::string logPath;
    std::string configPath;
    std::string buildId;
    std::string role;
};

bool initializeAuditLog(const AuditLogConfig &config, std::string &status);
bool logAuditEvent(const std::string &eventType, const std::string &message, const std::string &detail);
void setAuditRole(const std::string &role);
std::string auditLogStatus();
bool auditLogHealthy();
} // namespace ui

#endif // UI_AUDIT_LOG_H
