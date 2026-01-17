#include "core/logging.h"

namespace
{
LogSink *g_sink = nullptr;
}

void setLogSink(LogSink *sink)
{
    g_sink = sink;
}

void logMessage(LogLevel level, const std::string &message)
{
    if (!g_sink)
    {
        return;
    }
    g_sink->log(level, message);
}
