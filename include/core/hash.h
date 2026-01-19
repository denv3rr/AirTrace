#ifndef CORE_HASH_H
#define CORE_HASH_H

#include <string>
#include <vector>

std::string sha256Hex(const std::vector<unsigned char> &data);
bool hashEquals(const std::string &expectedHex, const std::string &actualHex);

#endif // CORE_HASH_H
