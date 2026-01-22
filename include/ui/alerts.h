#ifndef ALERTS_H
#define ALERTS_H

#include <string>

namespace ui
{
std::string denialRecoveryHint(const std::string &denialReason);
std::string buildDenialBanner(const std::string &denialReason);
} // namespace ui

#endif // ALERTS_H
