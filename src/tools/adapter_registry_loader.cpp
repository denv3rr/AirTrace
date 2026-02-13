#include "tools/adapter_registry_loader.h"

#include "core/adapter_registry.h"
#include "core/hash.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace tools
{
namespace
{
constexpr const char *kCoreVersion = "1.0.0";
constexpr const char *kToolsVersion = "1.0.0";
constexpr const char *kUiVersion = "1.0.0";
constexpr const char *kAdapterContractVersion = "1.0.0";
constexpr const char *kUiContractVersion = "1.0.0";

struct JsonValue
{
    enum class Type
    {
        Null,
        Bool,
        Number,
        String,
        Object,
        Array
    };

    Type type = Type::Null;
    bool boolValue = false;
    double numberValue = 0.0;
    std::string stringValue;
    std::vector<std::pair<std::string, JsonValue>> objectValue;
    std::vector<JsonValue> arrayValue;
};

struct JsonParser
{
    const std::string &text;
    size_t pos = 0;
    std::string error;

    bool parse(JsonValue &out)
    {
        skipWhitespace();
        if (!parseValue(out))
        {
            return false;
        }
        skipWhitespace();
        if (pos != text.size())
        {
            error = "trailing characters";
            return false;
        }
        return true;
    }

    void skipWhitespace()
    {
        while (pos < text.size() && std::isspace(static_cast<unsigned char>(text[pos])))
        {
            ++pos;
        }
    }

    bool parseValue(JsonValue &out)
    {
        if (pos >= text.size())
        {
            error = "unexpected end of input";
            return false;
        }
        char ch = text[pos];
        if (ch == '"')
        {
            out.type = JsonValue::Type::String;
            return parseString(out.stringValue);
        }
        if (ch == '{')
        {
            out.type = JsonValue::Type::Object;
            return parseObject(out);
        }
        if (ch == '[')
        {
            out.type = JsonValue::Type::Array;
            return parseArray(out);
        }
        if (ch == '-' || std::isdigit(static_cast<unsigned char>(ch)))
        {
            out.type = JsonValue::Type::Number;
            return parseNumber(out.numberValue);
        }
        if (text.compare(pos, 4, "true") == 0)
        {
            out.type = JsonValue::Type::Bool;
            out.boolValue = true;
            pos += 4;
            return true;
        }
        if (text.compare(pos, 5, "false") == 0)
        {
            out.type = JsonValue::Type::Bool;
            out.boolValue = false;
            pos += 5;
            return true;
        }
        if (text.compare(pos, 4, "null") == 0)
        {
            out.type = JsonValue::Type::Null;
            pos += 4;
            return true;
        }
        error = "unexpected token";
        return false;
    }

    bool parseString(std::string &out)
    {
        if (text[pos] != '"')
        {
            error = "expected string";
            return false;
        }
        ++pos;
        std::ostringstream result;
        while (pos < text.size())
        {
            char ch = text[pos++];
            if (ch == '"')
            {
                out = result.str();
                return true;
            }
            if (ch == '\\')
            {
                if (pos >= text.size())
                {
                    error = "incomplete escape";
                    return false;
                }
                char esc = text[pos++];
                switch (esc)
                {
                case '"': result << '"'; break;
                case '\\': result << '\\'; break;
                case '/': result << '/'; break;
                case 'b': result << '\b'; break;
                case 'f': result << '\f'; break;
                case 'n': result << '\n'; break;
                case 'r': result << '\r'; break;
                case 't': result << '\t'; break;
                default:
                    error = "unsupported escape";
                    return false;
                }
                continue;
            }
            result << ch;
        }
        error = "unterminated string";
        return false;
    }

    bool parseNumber(double &out)
    {
        const char *start = text.c_str() + pos;
        char *end = nullptr;
        out = std::strtod(start, &end);
        if (end == start)
        {
            error = "invalid number";
            return false;
        }
        pos += static_cast<size_t>(end - start);
        return true;
    }

    bool parseArray(JsonValue &out)
    {
        if (text[pos] != '[')
        {
            error = "expected array";
            return false;
        }
        ++pos;
        skipWhitespace();
        if (pos < text.size() && text[pos] == ']')
        {
            ++pos;
            return true;
        }
        while (pos < text.size())
        {
            JsonValue value;
            skipWhitespace();
            if (!parseValue(value))
            {
                return false;
            }
            out.arrayValue.push_back(std::move(value));
            skipWhitespace();
            if (pos >= text.size())
            {
                error = "unterminated array";
                return false;
            }
            if (text[pos] == ']')
            {
                ++pos;
                return true;
            }
            if (text[pos] != ',')
            {
                error = "expected ',' in array";
                return false;
            }
            ++pos;
        }
        error = "unterminated array";
        return false;
    }

    bool parseObject(JsonValue &out)
    {
        if (text[pos] != '{')
        {
            error = "expected object";
            return false;
        }
        ++pos;
        skipWhitespace();
        if (pos < text.size() && text[pos] == '}')
        {
            ++pos;
            return true;
        }
        while (pos < text.size())
        {
            skipWhitespace();
            JsonValue keyValue;
            if (!parseValue(keyValue) || keyValue.type != JsonValue::Type::String)
            {
                error = "expected string key";
                return false;
            }
            std::string key = keyValue.stringValue;
            skipWhitespace();
            if (pos >= text.size() || text[pos] != ':')
            {
                error = "expected ':' after key";
                return false;
            }
            ++pos;
            skipWhitespace();
            JsonValue value;
            if (!parseValue(value))
            {
                return false;
            }
            out.objectValue.emplace_back(std::move(key), std::move(value));
            skipWhitespace();
            if (pos >= text.size())
            {
                error = "unterminated object";
                return false;
            }
            if (text[pos] == '}')
            {
                ++pos;
                return true;
            }
            if (text[pos] != ',')
            {
                error = "expected ',' in object";
                return false;
            }
            ++pos;
        }
        error = "unterminated object";
        return false;
    }
};

const JsonValue *findKey(const JsonValue &object, const std::string &key)
{
    for (const auto &entry : object.objectValue)
    {
        if (entry.first == key)
        {
            return &entry.second;
        }
    }
    return nullptr;
}

bool readFile(const std::string &path, std::string &out)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        return false;
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    out = buffer.str();
    return true;
}

bool getStringField(const JsonValue &object, const std::string &key, std::string &out)
{
    const JsonValue *value = findKey(object, key);
    if (!value || value->type != JsonValue::Type::String)
    {
        return false;
    }
    out = value->stringValue;
    return true;
}

bool getNumberField(const JsonValue &object, const std::string &key, double &out)
{
    const JsonValue *value = findKey(object, key);
    if (!value || value->type != JsonValue::Type::Number)
    {
        return false;
    }
    out = value->numberValue;
    return true;
}

bool getArrayField(const JsonValue &object, const std::string &key, std::vector<JsonValue> &out)
{
    const JsonValue *value = findKey(object, key);
    if (!value || value->type != JsonValue::Type::Array)
    {
        return false;
    }
    out = value->arrayValue;
    return true;
}

std::string toLower(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

bool isLeapYear(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

int daysInMonth(int year, int month)
{
    static const int kDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && isLeapYear(year))
    {
        return 29;
    }
    return kDays[month - 1];
}

bool parseIsoDate(const std::string &value, int &year, int &month, int &day)
{
    if (value.size() != 10 || value[4] != '-' || value[7] != '-')
    {
        return false;
    }
    for (size_t idx = 0; idx < value.size(); ++idx)
    {
        if (idx == 4 || idx == 7)
        {
            continue;
        }
        if (!std::isdigit(static_cast<unsigned char>(value[idx])))
        {
            return false;
        }
    }
    year = (value[0] - '0') * 1000 +
           (value[1] - '0') * 100 +
           (value[2] - '0') * 10 +
           (value[3] - '0');
    month = (value[5] - '0') * 10 +
            (value[6] - '0');
    day = (value[8] - '0') * 10 +
          (value[9] - '0');
    if (year < 1970 || month < 1 || month > 12)
    {
        return false;
    }
    if (day < 1 || day > daysInMonth(year, month))
    {
        return false;
    }
    return true;
}

int daysFromCivil(int year, unsigned int month, unsigned int day)
{
    year -= month <= 2 ? 1 : 0;
    const int era = (year >= 0 ? year : year - 399) / 400;
    const unsigned int yoe = static_cast<unsigned int>(year - era * 400);
    const unsigned int doy = (153 * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const unsigned int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<int>(doe) - 719468;
}

bool approvalDateIsFresh(const std::string &approvalDate, int maxAgeDays, bool &stale)
{
    stale = false;
    int year = 0;
    int month = 0;
    int day = 0;
    if (!parseIsoDate(approvalDate, year, month, day))
    {
        return false;
    }

    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm *utc = std::gmtime(&now);
    if (!utc)
    {
        return false;
    }
    int todayDays = daysFromCivil(utc->tm_year + 1900, static_cast<unsigned int>(utc->tm_mon + 1), static_cast<unsigned int>(utc->tm_mday));
    int approvalDays = daysFromCivil(year, static_cast<unsigned int>(month), static_cast<unsigned int>(day));
    if (approvalDays > todayDays)
    {
        return false;
    }

    int ageDays = todayDays - approvalDays;
    stale = ageDays > maxAgeDays;
    return true;
}

bool parseManifest(const std::string &content, AdapterManifest &manifest, std::vector<AdapterUiField> &uiFields, std::string &error)
{
    JsonValue root;
    JsonParser parser{content};
    if (!parser.parse(root))
    {
        error = parser.error;
        return false;
    }
    if (root.type != JsonValue::Type::Object)
    {
        error = "manifest root not object";
        return false;
    }

    if (!getStringField(root, "adapter.id", manifest.adapterId) ||
        !getStringField(root, "adapter.version", manifest.adapterVersion) ||
        !getStringField(root, "adapter.contract_version", manifest.adapterContractVersion) ||
        !getStringField(root, "ui.contract_version", manifest.uiContractVersion) ||
        !getStringField(root, "core.compatibility.min", manifest.coreCompatibilityMin) ||
        !getStringField(root, "core.compatibility.max", manifest.coreCompatibilityMax) ||
        !getStringField(root, "tools.compatibility.min", manifest.toolsCompatibilityMin) ||
        !getStringField(root, "tools.compatibility.max", manifest.toolsCompatibilityMax) ||
        !getStringField(root, "ui.compatibility.min", manifest.uiCompatibilityMin) ||
        !getStringField(root, "ui.compatibility.max", manifest.uiCompatibilityMax))
    {
        error = "missing manifest fields";
        return false;
    }

    std::vector<JsonValue> capabilities;
    if (!getArrayField(root, "capabilities", capabilities))
    {
        error = "missing capabilities";
        return false;
    }
    for (const auto &entry : capabilities)
    {
        if (entry.type != JsonValue::Type::Object)
        {
            error = "invalid capability entry";
            return false;
        }
        AdapterCapability cap;
        if (!getStringField(entry, "id", cap.id) ||
            !getStringField(entry, "description", cap.description) ||
            !getStringField(entry, "units", cap.units) ||
            !getNumberField(entry, "range_min", cap.rangeMin) ||
            !getNumberField(entry, "range_max", cap.rangeMax) ||
            !getStringField(entry, "error_behavior", cap.errorBehavior))
        {
            error = "invalid capability fields";
            return false;
        }
        manifest.capabilities.push_back(cap);
    }

    std::vector<JsonValue> extensions;
    if (!getArrayField(root, "ui_extensions", extensions))
    {
        error = "missing ui_extensions";
        return false;
    }
    for (const auto &entry : extensions)
    {
        if (entry.type != JsonValue::Type::Object)
        {
            error = "invalid ui extension entry";
            return false;
        }
        AdapterUiExtension ext;
        AdapterUiField field;
        if (!getStringField(entry, "field_id", ext.fieldId) ||
            !getStringField(entry, "type", ext.type) ||
            !getStringField(entry, "units", ext.units) ||
            !getNumberField(entry, "range_min", ext.rangeMin) ||
            !getNumberField(entry, "range_max", ext.rangeMax) ||
            !getStringField(entry, "error_behavior", ext.errorBehavior))
        {
            error = "invalid ui extension fields";
            return false;
        }
        std::vector<JsonValue> surfaces;
        if (!getArrayField(entry, "surfaces", surfaces))
        {
            error = "missing ui extension surfaces";
            return false;
        }
        for (const auto &surfaceValue : surfaces)
        {
            if (surfaceValue.type != JsonValue::Type::String)
            {
                error = "invalid ui surface";
                return false;
            }
            ext.surfaces.push_back(toLower(surfaceValue.stringValue));
        }
        manifest.uiExtensions.push_back(ext);

        field.fieldId = ext.fieldId;
        field.type = ext.type;
        field.units = ext.units;
        field.rangeMin = ext.rangeMin;
        field.rangeMax = ext.rangeMax;
        field.errorBehavior = ext.errorBehavior;
        field.surfaces = ext.surfaces;
        uiFields.push_back(std::move(field));
    }

    return true;
}

bool parseAllowlist(const std::string &content, std::vector<AdapterAllowlistEntry> &entries, std::string &error)
{
    JsonValue root;
    JsonParser parser{content};
    if (!parser.parse(root))
    {
        error = parser.error;
        return false;
    }
    if (root.type != JsonValue::Type::Object)
    {
        error = "allowlist root not object";
        return false;
    }

    std::vector<JsonValue> items;
    if (!getArrayField(root, "entries", items))
    {
        error = "missing entries";
        return false;
    }
    for (const auto &entry : items)
    {
        if (entry.type != JsonValue::Type::Object)
        {
            error = "invalid entry";
            return false;
        }
        AdapterAllowlistEntry allowlist;
        if (!getStringField(entry, "adapter.id", allowlist.adapterId) ||
            !getStringField(entry, "adapter.version", allowlist.adapterVersion) ||
            !getStringField(entry, "signature.hash", allowlist.signatureHash) ||
            !getStringField(entry, "signature.algorithm", allowlist.signatureAlgorithm) ||
            !getStringField(entry, "approved_by", allowlist.approvedBy) ||
            !getStringField(entry, "approval_date", allowlist.approvalDate))
        {
            error = "invalid allowlist fields";
            return false;
        }
        std::vector<JsonValue> surfaces;
        if (!getArrayField(entry, "allowed_surfaces", surfaces))
        {
            error = "invalid allowlist surfaces";
            return false;
        }
        for (const auto &surfaceValue : surfaces)
        {
            if (surfaceValue.type != JsonValue::Type::String)
            {
                error = "invalid allowlist surface";
                return false;
            }
            allowlist.allowedSurfaces.push_back(toLower(surfaceValue.stringValue));
        }
        entries.push_back(std::move(allowlist));
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

bool isOfficialAdapter(const std::string &adapterId)
{
    return adapterId == "air" || adapterId == "ground" || adapterId == "maritime" ||
           adapterId == "space" || adapterId == "handheld" || adapterId == "fixed_site" ||
           adapterId == "subsea";
}

std::string defaultManifestPath(const std::string &adapterId)
{
    if (adapterId.empty())
    {
        return "";
    }
    std::filesystem::path path = std::filesystem::path("adapters") / "official" / adapterId / "manifest.json";
    return path.string();
}

std::string defaultAllowlistPath()
{
    std::filesystem::path path = std::filesystem::path("adapters") / "allowlist.json";
    return path.string();
}

std::string computeHash(const std::string &content)
{
    std::vector<unsigned char> data(content.begin(), content.end());
    return sha256Hex(data);
}

std::vector<AdapterUiField> filterFieldsForSurface(const std::vector<AdapterUiField> &fields, const std::string &surface)
{
    std::vector<AdapterUiField> filtered;
    for (const auto &field : fields)
    {
        if (hasSurface(field.surfaces, surface))
        {
            filtered.push_back(field);
        }
    }
    return filtered;
}
} // namespace

AdapterUiSnapshot loadAdapterUiSnapshot(const SimConfig &config)
{
    AdapterUiSnapshot snapshot;
    snapshot.adapterId = config.adapter.id;
    snapshot.adapterVersion = config.adapter.version;
    snapshot.surface = config.adapter.uiSurface.empty() ? "tui" : toLower(config.adapter.uiSurface);
    snapshot.status = "none";
    snapshot.reason = "none";
    snapshot.approvedBy = "";
    snapshot.approvalDate = "";
    snapshot.signatureAlgorithm = "";
    snapshot.contextVersionSummary = "";

    if (snapshot.adapterId.empty())
    {
        return snapshot;
    }

    std::string manifestPath = config.adapter.manifestPath;
    if (manifestPath.empty() && isOfficialAdapter(snapshot.adapterId))
    {
        manifestPath = defaultManifestPath(snapshot.adapterId);
    }
    std::string allowlistPath = config.adapter.allowlistPath;
    if (allowlistPath.empty())
    {
        allowlistPath = defaultAllowlistPath();
    }

    if (manifestPath.empty())
    {
        snapshot.status = "manifest_missing";
        snapshot.reason = "adapter_manifest_missing";
        return snapshot;
    }
    if (allowlistPath.empty())
    {
        snapshot.status = "allowlist_missing";
        snapshot.reason = "adapter_allowlist_missing";
        return snapshot;
    }

    std::string manifestContent;
    if (!readFile(manifestPath, manifestContent))
    {
        snapshot.status = "manifest_missing";
        snapshot.reason = "adapter_manifest_missing";
        return snapshot;
    }

    AdapterManifest manifest;
    std::vector<AdapterUiField> uiFields;
    std::string parseError;
    if (!parseManifest(manifestContent, manifest, uiFields, parseError))
    {
        snapshot.status = "manifest_invalid";
        snapshot.reason = "adapter_manifest_invalid";
        return snapshot;
    }

    if (manifest.adapterId != snapshot.adapterId || manifest.adapterVersion != snapshot.adapterVersion)
    {
        snapshot.status = "manifest_invalid";
        snapshot.reason = "adapter_schema_invalid";
        return snapshot;
    }

    std::string allowlistContent;
    if (!readFile(allowlistPath, allowlistContent))
    {
        snapshot.status = "allowlist_missing";
        snapshot.reason = "adapter_allowlist_missing";
        return snapshot;
    }

    std::vector<AdapterAllowlistEntry> allowlistEntries;
    if (!parseAllowlist(allowlistContent, allowlistEntries, parseError))
    {
        snapshot.status = "allowlist_invalid";
        snapshot.reason = "adapter_allowlist_invalid";
        return snapshot;
    }

    AdapterAllowlistEntry selectedEntry;
    bool found = false;
    for (const auto &entry : allowlistEntries)
    {
        if (entry.adapterId == manifest.adapterId && entry.adapterVersion == manifest.adapterVersion)
        {
            selectedEntry = entry;
            found = true;
            break;
        }
    }
    if (!found)
    {
        snapshot.status = "not_allowlisted";
        snapshot.reason = "adapter_not_allowlisted";
        return snapshot;
    }

    snapshot.approvedBy = selectedEntry.approvedBy;
    snapshot.approvalDate = selectedEntry.approvalDate;
    snapshot.signatureAlgorithm = selectedEntry.signatureAlgorithm;

    bool stale = false;
    if (!approvalDateIsFresh(selectedEntry.approvalDate, config.adapter.allowlistMaxAgeDays, stale))
    {
        snapshot.status = "allowlist_invalid";
        snapshot.reason = "adapter_allowlist_invalid";
        return snapshot;
    }
    if (stale)
    {
        snapshot.status = "allowlist_stale";
        snapshot.reason = "adapter_allowlist_stale";
        return snapshot;
    }

    if (toLower(selectedEntry.signatureAlgorithm) != "sha256")
    {
        snapshot.status = "signature_invalid";
        snapshot.reason = "adapter_signature_invalid";
        return snapshot;
    }

    std::string manifestHash = computeHash(manifestContent);
    if (!hashEquals(selectedEntry.signatureHash, manifestHash))
    {
        snapshot.status = "signature_invalid";
        snapshot.reason = "adapter_signature_invalid";
        return snapshot;
    }

    AdapterRegistryContext context;
    context.coreVersion = config.adapter.coreVersion.empty() ? kCoreVersion : config.adapter.coreVersion;
    context.toolsVersion = config.adapter.toolsVersion.empty() ? kToolsVersion : config.adapter.toolsVersion;
    context.uiVersion = config.adapter.uiVersion.empty() ? kUiVersion : config.adapter.uiVersion;
    context.adapterContractVersion = config.adapter.adapterContractVersion.empty() ? kAdapterContractVersion : config.adapter.adapterContractVersion;
    context.uiContractVersion = config.adapter.uiContractVersion.empty() ? kUiContractVersion : config.adapter.uiContractVersion;
    snapshot.contextVersionSummary = "core=" + context.coreVersion +
                                     ",tools=" + context.toolsVersion +
                                     ",ui=" + context.uiVersion +
                                     ",adapter_contract=" + context.adapterContractVersion +
                                     ",ui_contract=" + context.uiContractVersion;

    AdapterRegistryResult registryResult;
    if (!validateAdapterRegistration(manifest, selectedEntry, context, snapshot.surface, registryResult))
    {
        snapshot.status = "rejected";
        snapshot.reason = registryResult.reason;
        return snapshot;
    }

    snapshot.fields = filterFieldsForSurface(uiFields, snapshot.surface);
    snapshot.status = "ok";
    snapshot.reason = "ok";
    return snapshot;
}

bool validateAdapterSelection(const SimConfig &config, std::string &reason)
{
    AdapterUiSnapshot snapshot = loadAdapterUiSnapshot(config);
    if (snapshot.status == "none")
    {
        return true;
    }
    if (snapshot.status != "ok")
    {
        reason = snapshot.reason;
        return false;
    }
    return true;
}
} // namespace tools
