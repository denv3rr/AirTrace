#ifndef CORE_LOGGING_H
#define CORE_LOGGING_H

#include <string>

enum class LogLevel
{
    Info,
    Warning,
    Error
};

class LogSink
{
public:
    virtual ~LogSink() = default;
    virtual void log(LogLevel level, const std::string &message) = 0;
};

void setLogSink(LogSink *sink);
void logMessage(LogLevel level, const std::string &message);

#endif // CORE_LOGGING_H
