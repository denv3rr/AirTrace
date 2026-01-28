#include "ui/audit_log.h"

#include "core/hash.h"
#include "core/logging.h"

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <vector>

namespace ui
{
namespace
{
constexpr std::size_t kMaxAuditBytes = 5 * 1024 * 1024;

struct AuditLogState
{
    std::string path;
    std::string buildId;
    std::string configId;
    std::string role;
    std::string lastHash;
    std::string status = "uninitialized";
    bool healthy = false;
};

AuditLogState g_state{};
std::mutex g_mutex;

std::string escapeJson(const std::string &value)
{
    std::ostringstream out;
    for (char ch : value)
    {
        switch (ch)
        {
        case '"': out << "\\\""; break;
        case '\\': out << "\\\\"; break;
        case '\n': out << "\\n"; break;
        case '\r': out << "\\r"; break;
        case '\t': out << "\\t"; break;
        default:
            if (static_cast<unsigned char>(ch) < 0x20)
            {
                out << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<int>(static_cast<unsigned char>(ch));
            }
            else
            {
                out << ch;
            }
            break;
        }
    }
    return out.str();
}

std::string utcTimestamp()
{
    using namespace std::chrono;
    auto now = system_clock::now();
    std::time_t tt = system_clock::to_time_t(now);
    std::tm tm{};
    gmtime_s(&tm, &tt);
    std::ostringstream out;
    out << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    return out.str();
}

std::string hashFileHex(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return "unavailable";
    }
    std::vector<unsigned char> data((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return sha256Hex(data);
}

bool ensureCapacity(const std::string &path)
{
    std::error_code ec;
    if (std::filesystem::exists(path, ec))
    {
        auto size = std::filesystem::file_size(path, ec);
        if (!ec && size >= kMaxAuditBytes)
        {
            return false;
        }
    }
    return true;
}

std::string makeRecord(const std::string &eventType, const std::string &message, const std::string &detail, const std::string &timestamp)
{
    std::ostringstream payload;
    payload << eventType << "|" << message << "|" << detail << "|" << timestamp << "|"
            << g_state.buildId << "|" << g_state.configId << "|" << g_state.role << "|" << g_state.lastHash;
    std::string payloadStr = payload.str();
    std::vector<unsigned char> data(payloadStr.begin(), payloadStr.end());
    std::string entryHash = sha256Hex(data);

    std::ostringstream out;
    out << "{\"ts\":\"" << escapeJson(timestamp) << "\","
        << "\"event\":\"" << escapeJson(eventType) << "\","
        << "\"message\":\"" << escapeJson(message) << "\","
        << "\"detail\":\"" << escapeJson(detail) << "\","
        << "\"build_id\":\"" << escapeJson(g_state.buildId) << "\","
        << "\"config_id\":\"" << escapeJson(g_state.configId) << "\","
        << "\"role\":\"" << escapeJson(g_state.role) << "\","
        << "\"prev_hash\":\"" << escapeJson(g_state.lastHash) << "\","
        << "\"entry_hash\":\"" << escapeJson(entryHash) << "\"}"
        << "\n";
    g_state.lastHash = entryHash;
    return out.str();
}

class AuditLogSink final : public LogSink
{
public:
    void log(LogLevel level, const std::string &message) override
    {
        const char *levelText = (level == LogLevel::Error) ? "error" : (level == LogLevel::Warning ? "warning" : "info");
        logAuditEvent("core_log", levelText, message);
    }
};

AuditLogSink g_sink{};
} // namespace

bool initializeAuditLog(const AuditLogConfig &config, std::string &status)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    g_state.path = config.logPath;
    g_state.buildId = config.buildId.empty() ? "unknown" : config.buildId;
    g_state.configId = hashFileHex(config.configPath);
    g_state.role = config.role.empty() ? "unknown" : config.role;
    g_state.lastHash.clear();

    if (!ensureCapacity(g_state.path))
    {
        g_state.healthy = false;
        g_state.status = "retention_exceeded";
        status = g_state.status;
        setLogSink(nullptr);
        return false;
    }

    std::ofstream out(g_state.path, std::ios::app);
    if (!out)
    {
        g_state.healthy = false;
        g_state.status = "unavailable";
        status = g_state.status;
        setLogSink(nullptr);
        return false;
    }

    g_state.healthy = true;
    g_state.status = "ok";
    status = g_state.status;
    setLogSink(&g_sink);

    const std::string timestamp = utcTimestamp();
    std::string record = makeRecord("audit_start", "audit log initialized", "", timestamp);
    out << record;
    return true;
}

bool logAuditEvent(const std::string &eventType, const std::string &message, const std::string &detail)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!g_state.healthy)
    {
        return false;
    }
    if (!ensureCapacity(g_state.path))
    {
        g_state.healthy = false;
        g_state.status = "retention_exceeded";
        return false;
    }
    std::ofstream out(g_state.path, std::ios::app);
    if (!out)
    {
        g_state.healthy = false;
        g_state.status = "write_failed";
        return false;
    }

    const std::string timestamp = utcTimestamp();
    std::string record = makeRecord(eventType, message, detail, timestamp);
    out << record;
    return true;
}

void setAuditRole(const std::string &role)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (!role.empty())
    {
        g_state.role = role;
    }
}

std::string auditLogStatus()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_state.status;
}

bool auditLogHealthy()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    return g_state.healthy;
}
} // namespace ui
