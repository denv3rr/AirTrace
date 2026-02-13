#ifndef CORE_PLUGIN_AUTH_H
#define CORE_PLUGIN_AUTH_H

#include <string>

struct PluginIdentity
{
    std::string id;
    std::string version;
    bool deviceDriver = false;
};

struct PluginSignature
{
    std::string hash;
    std::string algorithm;
};

struct PluginAllowlist
{
    std::string id;
    std::string version;
    std::string signatureHash;
    std::string signatureAlgorithm;
};

struct PluginAuthorization
{
    bool required = true;
    bool granted = false;
};

struct PluginAuthRequest
{
    PluginIdentity identity;
    PluginSignature signature;
    PluginAllowlist allowlist;
    PluginAuthorization authorization;
};

struct PluginAuthResult
{
    bool allowed = false;
    std::string reason = "plugin_not_authorized";
};

bool validatePluginActivation(const PluginAuthRequest &request, PluginAuthResult &result);

#endif // CORE_PLUGIN_AUTH_H
