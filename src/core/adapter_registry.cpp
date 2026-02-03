#include "core/adapter_registry.h"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace
{
bool isValidId(const std::string &value)
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

bool isValidSurface(const std::string &value)
{
    return value == "tui" || value == "cockpit" || value == "remote_operator" || value == "c2";
}

bool parseSemver(const std::string &value, int &major, int &minor, int &patch)
{
    char dot1 = '\0';
    char dot2 = '\0';
    std::istringstream in(value);
    if (!(in >> major >> dot1 >> minor >> dot2 >> patch))
    {
        return false;
    }
    if (dot1 != '.' || dot2 != '.')
    {
        return false;
    }
    return true;
}

int compareSemver(const std::string &lhs, const std::string &rhs, bool &ok)
{
    int lmajor = 0;
    int lminor = 0;
    int lpatch = 0;
    int rmajor = 0;
    int rminor = 0;
    int rpatch = 0;
    if (!parseSemver(lhs, lmajor, lminor, lpatch) || !parseSemver(rhs, rmajor, rminor, rpatch))
    {
        ok = false;
        return 0;
    }
    ok = true;
    if (lmajor != rmajor)
    {
        return (lmajor < rmajor) ? -1 : 1;
    }
    if (lminor != rminor)
    {
        return (lminor < rminor) ? -1 : 1;
    }
    if (lpatch != rpatch)
    {
        return (lpatch < rpatch) ? -1 : 1;
    }
    return 0;
}

bool versionInRange(const std::string &value, const std::string &minValue, const std::string &maxValue)
{
    bool ok = false;
    int cmpMin = compareSemver(value, minValue, ok);
    if (!ok || cmpMin < 0)
    {
        return false;
    }
    int cmpMax = compareSemver(value, maxValue, ok);
    if (!ok || cmpMax > 0)
    {
        return false;
    }
    return true;
}

bool hasSurface(const std::vector<std::string> &surfaces, const std::string &surface)
{
    for (const auto &entry : surfaces)
    {
        if (entry == surface)
        {
            return true;
        }
    }
    return false;
}

bool validateCapabilities(const std::vector<AdapterCapability> &capabilities)
{
    for (const auto &cap : capabilities)
    {
        if (cap.id.empty() || cap.description.empty() || cap.errorBehavior.empty())
        {
            return false;
        }
        if (!isValidId(cap.id))
        {
            return false;
        }
        if (cap.rangeMin > cap.rangeMax)
        {
            return false;
        }
    }
    return true;
}

bool validateUiExtensions(const std::vector<AdapterUiExtension> &extensions)
{
    for (const auto &entry : extensions)
    {
        if (entry.fieldId.empty() || entry.type.empty() || entry.errorBehavior.empty())
        {
            return false;
        }
        if (entry.rangeMin > entry.rangeMax)
        {
            return false;
        }
        if (entry.surfaces.empty())
        {
            return false;
        }
        for (const auto &surface : entry.surfaces)
        {
            if (!isValidSurface(surface))
            {
                return false;
            }
        }
    }
    return true;
}
} // namespace

bool validateAdapterRegistration(const AdapterManifest &manifest,
                                 const AdapterAllowlistEntry &allowlist,
                                 const AdapterRegistryContext &context,
                                 const std::string &requestedSurface,
                                 AdapterRegistryResult &result)
{
    result.ok = false;
    result.reason = "adapter_schema_invalid";

    if (manifest.adapterId.empty() || manifest.adapterVersion.empty() ||
        manifest.adapterContractVersion.empty() || manifest.uiContractVersion.empty() ||
        manifest.coreCompatibilityMin.empty() || manifest.coreCompatibilityMax.empty() ||
        manifest.toolsCompatibilityMin.empty() || manifest.toolsCompatibilityMax.empty() ||
        manifest.uiCompatibilityMin.empty() || manifest.uiCompatibilityMax.empty())
    {
        return false;
    }
    if (!isValidId(manifest.adapterId) || !isValidId(allowlist.adapterId))
    {
        return false;
    }
    if (allowlist.adapterId.empty() || allowlist.adapterVersion.empty())
    {
        result.reason = "adapter_not_allowlisted";
        return false;
    }
    if (allowlist.adapterId != manifest.adapterId || allowlist.adapterVersion != manifest.adapterVersion)
    {
        result.reason = "adapter_not_allowlisted";
        return false;
    }
    if (!isValidSurface(requestedSurface))
    {
        result.reason = "adapter_ui_extension_invalid";
        return false;
    }
    if (!allowlist.allowedSurfaces.empty() && !hasSurface(allowlist.allowedSurfaces, requestedSurface))
    {
        result.reason = "adapter_surface_not_allowed";
        return false;
    }
    if (manifest.adapterContractVersion != context.adapterContractVersion ||
        manifest.uiContractVersion != context.uiContractVersion)
    {
        result.reason = "adapter_contract_mismatch";
        return false;
    }
    if (!versionInRange(context.coreVersion, manifest.coreCompatibilityMin, manifest.coreCompatibilityMax) ||
        !versionInRange(context.toolsVersion, manifest.toolsCompatibilityMin, manifest.toolsCompatibilityMax) ||
        !versionInRange(context.uiVersion, manifest.uiCompatibilityMin, manifest.uiCompatibilityMax))
    {
        result.reason = "adapter_version_incompatible";
        return false;
    }
    if (!validateCapabilities(manifest.capabilities))
    {
        result.reason = "adapter_capability_invalid";
        return false;
    }
    if (!validateUiExtensions(manifest.uiExtensions))
    {
        result.reason = "adapter_ui_extension_invalid";
        return false;
    }

    result.ok = true;
    result.reason = "ok";
    return true;
}
