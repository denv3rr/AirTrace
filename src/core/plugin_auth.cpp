#include "core/plugin_auth.h"

#include <cctype>
#include <sstream>

#include "core/hash.h"

namespace
{
std::string toLower(std::string value)
{
    for (char &ch : value)
    {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return value;
}

bool isValidIdentifier(const std::string &value)
{
    if (value.empty())
    {
        return false;
    }
    for (char ch : value)
    {
        if (!(std::islower(static_cast<unsigned char>(ch)) || std::isdigit(static_cast<unsigned char>(ch)) || ch == '_' || ch == '-'))
        {
            return false;
        }
    }
    return true;
}

bool isSemver(const std::string &value)
{
    int major = 0;
    int minor = 0;
    int patch = 0;
    char dot1 = '\0';
    char dot2 = '\0';
    std::istringstream input(value);
    if (!(input >> major >> dot1 >> minor >> dot2 >> patch))
    {
        return false;
    }
    if (dot1 != '.' || dot2 != '.')
    {
        return false;
    }
    if (major < 0 || minor < 0 || patch < 0)
    {
        return false;
    }
    char trailing = '\0';
    if (input >> trailing)
    {
        return false;
    }
    return true;
}

bool isSha256Hex(const std::string &value)
{
    if (value.size() != 64)
    {
        return false;
    }
    for (char ch : value)
    {
        if (!std::isdigit(static_cast<unsigned char>(ch)) &&
            !(ch >= 'a' && ch <= 'f') &&
            !(ch >= 'A' && ch <= 'F'))
        {
            return false;
        }
    }
    return true;
}
} // namespace

bool validatePluginActivation(const PluginAuthRequest &request, PluginAuthResult &result)
{
    result.allowed = false;
    result.reason = "plugin_not_authorized";

    if (!isValidIdentifier(request.identity.id))
    {
        result.reason = "plugin_id_invalid";
        return false;
    }
    if (!isSemver(request.identity.version))
    {
        result.reason = "plugin_version_invalid";
        return false;
    }
    if (!request.authorization.required)
    {
        result.reason = "plugin_not_authorized";
        return false;
    }
    if (!request.authorization.granted)
    {
        result.reason = "plugin_not_authorized";
        return false;
    }

    if (!isValidIdentifier(request.allowlist.id) || !isSemver(request.allowlist.version))
    {
        result.reason = "plugin_not_allowlisted";
        return false;
    }
    if (request.allowlist.id != request.identity.id || request.allowlist.version != request.identity.version)
    {
        result.reason = "plugin_not_allowlisted";
        return false;
    }

    const std::string pluginAlgorithm = toLower(request.signature.algorithm);
    const std::string allowlistAlgorithm = toLower(request.allowlist.signatureAlgorithm);
    if (pluginAlgorithm != "sha256" || allowlistAlgorithm != "sha256")
    {
        result.reason = "plugin_signature_invalid";
        return false;
    }
    if (!isSha256Hex(request.signature.hash) || !isSha256Hex(request.allowlist.signatureHash))
    {
        result.reason = "plugin_signature_invalid";
        return false;
    }
    if (!hashEquals(request.signature.hash, request.allowlist.signatureHash))
    {
        result.reason = "plugin_signature_invalid";
        return false;
    }

    result.allowed = true;
    result.reason = "ok";
    return true;
}
