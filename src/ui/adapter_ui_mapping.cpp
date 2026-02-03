#include "ui/adapter_ui_mapping.h"

#include <iomanip>
#include <sstream>

namespace ui
{
std::string formatAdapterFieldSummary(const std::vector<tools::AdapterUiField> &fields)
{
    if (fields.empty())
    {
        return "";
    }
    std::ostringstream out;
    for (size_t idx = 0; idx < fields.size(); ++idx)
    {
        const auto &field = fields[idx];
        out << field.fieldId;
        if (!field.units.empty())
        {
            out << "(" << field.units << ")";
        }
        out << "[";
        out << std::fixed << std::setprecision(2) << field.rangeMin << ".." << field.rangeMax << "]";
        if (!field.errorBehavior.empty())
        {
            out << "{err=" << field.errorBehavior << "}";
        }
        if (idx + 1 < fields.size())
        {
            out << ";";
        }
    }
    return out.str();
}
} // namespace ui
