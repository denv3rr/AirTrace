#include "tools/io_packager.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace
{
void printUsage()
{
    std::cerr << "Usage: AirTraceIoPackager [--list-formats] "
                 "--in-format <format> --out-format <format> "
                 "--input <path|-> --output <path|->\n";
}

void printFormats()
{
    const std::vector<tools::IoEnvelopeCodecDescriptor> codecs = tools::listIoEnvelopeCodecs();
    std::cout << "Supported formats:\n";
    for (const auto &codec : codecs)
    {
        std::cout << "  " << codec.canonicalName;
        if (!codec.aliases.empty())
        {
            std::cout << " (aliases: ";
            for (std::size_t idx = 0; idx < codec.aliases.size(); ++idx)
            {
                std::cout << codec.aliases[idx];
                if (idx + 1 < codec.aliases.size())
                {
                    std::cout << ",";
                }
            }
            std::cout << ")";
        }
        std::cout << "\n";
    }
}

bool readAll(const std::string &path, std::string &content)
{
    if (path == "-")
    {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        content = buffer.str();
        return true;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    content = buffer.str();
    return true;
}

bool writeAll(const std::string &path, const std::string &content)
{
    if (path == "-")
    {
        std::cout << content;
        return static_cast<bool>(std::cout);
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        return false;
    }
    file << content;
    return static_cast<bool>(file);
}
} // namespace

int main(int argc, char **argv)
{
    std::string inputPath = "-";
    std::string outputPath = "-";
    std::string inputFormatText;
    std::string outputFormatText;
    bool listFormats = false;

    for (int idx = 1; idx < argc; ++idx)
    {
        const std::string arg = argv[idx];
        if (arg == "--list-formats")
        {
            listFormats = true;
            continue;
        }
        if (arg == "--input" && idx + 1 < argc)
        {
            inputPath = argv[++idx];
            continue;
        }
        if (arg == "--output" && idx + 1 < argc)
        {
            outputPath = argv[++idx];
            continue;
        }
        if (arg == "--in-format" && idx + 1 < argc)
        {
            inputFormatText = argv[++idx];
            continue;
        }
        if (arg == "--out-format" && idx + 1 < argc)
        {
            outputFormatText = argv[++idx];
            continue;
        }
        printUsage();
        return 1;
    }

    if (listFormats)
    {
        printFormats();
        return 0;
    }

    if (inputFormatText.empty() || outputFormatText.empty())
    {
        printUsage();
        return 1;
    }

    if (!tools::isSupportedIoEnvelopeFormat(inputFormatText))
    {
        std::cerr << "Unsupported input format: " << inputFormatText << "\n";
        printFormats();
        return 1;
    }
    if (!tools::isSupportedIoEnvelopeFormat(outputFormatText))
    {
        std::cerr << "Unsupported output format: " << outputFormatText << "\n";
        printFormats();
        return 1;
    }

    std::string inputPayload;
    if (!readAll(inputPath, inputPayload))
    {
        std::cerr << "Failed to read input payload: " << inputPath << "\n";
        return 1;
    }

    const tools::IoEnvelopeSerializeResult converted =
        tools::convertExternalIoEnvelope(inputPayload, inputFormatText, outputFormatText);
    if (!converted.ok)
    {
        std::cerr << "Conversion failed: " << converted.error << "\n";
        return 1;
    }

    if (!writeAll(outputPath, converted.payload))
    {
        std::cerr << "Failed to write output payload: " << outputPath << "\n";
        return 1;
    }

    return 0;
}
