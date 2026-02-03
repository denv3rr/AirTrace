#ifndef TOOLS_AUDIT_LOG_H
#define TOOLS_AUDIT_LOG_H

#include <string>

namespace tools
{
struct AuditLogConfig
{
    std::string logPath;
    std::string configPath;
    std::string buildId;
    std::string role;
    std::string runId;
    std::string configVersion;
    unsigned int seed = 0;
};

bool initializeAuditLog(const AuditLogConfig &config, std::string &status);
bool logAuditEvent(const std::string &eventType, const std::string &message, const std::string &detail);
void setAuditRole(const std::string &role);
void setAuditRunContext(const std::string &runId, const std::string &configVersion, unsigned int seed);
std::string auditLogStatus();
bool auditLogHealthy();
} // namespace tools

#endif // TOOLS_AUDIT_LOG_H
