#ifndef TOOLS_IO_PACKAGER_H
#define TOOLS_IO_PACKAGER_H

#include <string>
#include <vector>

#include "core/external_io_envelope.h"

namespace tools
{
enum class IoEnvelopeFormat
{
    Json,
    KeyValue
};

struct IoEnvelopeParseResult
{
    bool ok = false;
    std::string error;
    ExternalIoEnvelope envelope{};
};

struct IoEnvelopeSerializeResult
{
    bool ok = false;
    std::string error;
    std::string payload;
};

struct IoEnvelopeCodecDescriptor
{
    std::string canonicalName;
    std::vector<std::string> aliases;
};

std::vector<IoEnvelopeCodecDescriptor> listIoEnvelopeCodecs();
bool isSupportedIoEnvelopeFormat(const std::string &text);
bool parseIoEnvelopeFormat(const std::string &text, IoEnvelopeFormat &format);
std::string ioEnvelopeFormatName(IoEnvelopeFormat format);

IoEnvelopeParseResult parseExternalIoEnvelope(const std::string &formatName, const std::string &payload);
IoEnvelopeParseResult parseExternalIoEnvelope(IoEnvelopeFormat format, const std::string &payload);
IoEnvelopeSerializeResult serializeExternalIoEnvelope(const std::string &formatName, const ExternalIoEnvelope &envelope);
IoEnvelopeSerializeResult serializeExternalIoEnvelope(IoEnvelopeFormat format, const ExternalIoEnvelope &envelope);
IoEnvelopeSerializeResult convertExternalIoEnvelope(
    const std::string &payload,
    const std::string &inputFormatName,
    const std::string &outputFormatName);
IoEnvelopeSerializeResult convertExternalIoEnvelope(
    const std::string &payload,
    IoEnvelopeFormat inputFormat,
    IoEnvelopeFormat outputFormat);
} // namespace tools

#endif // TOOLS_IO_PACKAGER_H
