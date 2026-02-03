#ifndef UI_ADAPTER_UI_MAPPING_H
#define UI_ADAPTER_UI_MAPPING_H

#include <string>
#include <vector>

#include "tools/adapter_registry_loader.h"

namespace ui
{
std::string formatAdapterFieldSummary(const std::vector<tools::AdapterUiField> &fields);
} // namespace ui

#endif // UI_ADAPTER_UI_MAPPING_H
