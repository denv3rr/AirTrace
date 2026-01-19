#include "core/hash.h"

#include <array>
#include <cstdint>
#include <iomanip>
#include <sstream>

namespace
{
constexpr std::array<uint32_t, 64> kTable = {
    0x428a2f98U, 0x71374491U, 0xb5c0fbcfU, 0xe9b5dba5U,
    0x3956c25bU, 0x59f111f1U, 0x923f82a4U, 0xab1c5ed5U,
    0xd807aa98U, 0x12835b01U, 0x243185beU, 0x550c7dc3U,
    0x72be5d74U, 0x80deb1feU, 0x9bdc06a7U, 0xc19bf174U,
    0xe49b69c1U, 0xefbe4786U, 0x0fc19dc6U, 0x240ca1ccU,
    0x2de92c6fU, 0x4a7484aaU, 0x5cb0a9dcU, 0x76f988daU,
    0x983e5152U, 0xa831c66dU, 0xb00327c8U, 0xbf597fc7U,
    0xc6e00bf3U, 0xd5a79147U, 0x06ca6351U, 0x14292967U,
    0x27b70a85U, 0x2e1b2138U, 0x4d2c6dfcU, 0x53380d13U,
    0x650a7354U, 0x766a0abbU, 0x81c2c92eU, 0x92722c85U,
    0xa2bfe8a1U, 0xa81a664bU, 0xc24b8b70U, 0xc76c51a3U,
    0xd192e819U, 0xd6990624U, 0xf40e3585U, 0x106aa070U,
    0x19a4c116U, 0x1e376c08U, 0x2748774cU, 0x34b0bcb5U,
    0x391c0cb3U, 0x4ed8aa4aU, 0x5b9cca4fU, 0x682e6ff3U,
    0x748f82eeU, 0x78a5636fU, 0x84c87814U, 0x8cc70208U,
    0x90befffaU, 0xa4506cebU, 0xbef9a3f7U, 0xc67178f2U};

uint32_t rotr(uint32_t value, uint32_t bits)
{
    return (value >> bits) | (value << (32U - bits));
}

uint32_t ch(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (~x & z);
}

uint32_t maj(uint32_t x, uint32_t y, uint32_t z)
{
    return (x & y) ^ (x & z) ^ (y & z);
}

uint32_t bigSigma0(uint32_t x)
{
    return rotr(x, 2U) ^ rotr(x, 13U) ^ rotr(x, 22U);
}

uint32_t bigSigma1(uint32_t x)
{
    return rotr(x, 6U) ^ rotr(x, 11U) ^ rotr(x, 25U);
}

uint32_t smallSigma0(uint32_t x)
{
    return rotr(x, 7U) ^ rotr(x, 18U) ^ (x >> 3U);
}

uint32_t smallSigma1(uint32_t x)
{
    return rotr(x, 17U) ^ rotr(x, 19U) ^ (x >> 10U);
}

std::string toHex(const std::array<uint32_t, 8> &state)
{
    std::ostringstream out;
    out << std::hex << std::setfill('0');
    for (uint32_t value : state)
    {
        out << std::setw(8) << value;
    }
    return out.str();
}
} // namespace

std::string sha256Hex(const std::vector<unsigned char> &data)
{
    std::array<uint32_t, 8> state = {
        0x6a09e667U,
        0xbb67ae85U,
        0x3c6ef372U,
        0xa54ff53aU,
        0x510e527fU,
        0x9b05688cU,
        0x1f83d9abU,
        0x5be0cd19U};

    std::vector<unsigned char> buffer = data;
    uint64_t bitLen = static_cast<uint64_t>(buffer.size()) * 8U;
    buffer.push_back(0x80U);
    while ((buffer.size() % 64U) != 56U)
    {
        buffer.push_back(0x00U);
    }
    for (int i = 7; i >= 0; --i)
    {
        buffer.push_back(static_cast<unsigned char>((bitLen >> (i * 8)) & 0xFFU));
    }

    std::array<uint32_t, 64> w{};
    for (size_t offset = 0; offset < buffer.size(); offset += 64U)
    {
        for (size_t i = 0; i < 16U; ++i)
        {
            size_t index = offset + i * 4U;
            w[i] = (static_cast<uint32_t>(buffer[index]) << 24U) |
                   (static_cast<uint32_t>(buffer[index + 1]) << 16U) |
                   (static_cast<uint32_t>(buffer[index + 2]) << 8U) |
                   (static_cast<uint32_t>(buffer[index + 3]));
        }
        for (size_t i = 16U; i < 64U; ++i)
        {
            w[i] = smallSigma1(w[i - 2]) + w[i - 7] + smallSigma0(w[i - 15]) + w[i - 16];
        }

        uint32_t a = state[0];
        uint32_t b = state[1];
        uint32_t c = state[2];
        uint32_t d = state[3];
        uint32_t e = state[4];
        uint32_t f = state[5];
        uint32_t g = state[6];
        uint32_t h = state[7];

        for (size_t i = 0; i < 64U; ++i)
        {
            uint32_t temp1 = h + bigSigma1(e) + ch(e, f, g) + kTable[i] + w[i];
            uint32_t temp2 = bigSigma0(a) + maj(a, b, c);
            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        state[0] += a;
        state[1] += b;
        state[2] += c;
        state[3] += d;
        state[4] += e;
        state[5] += f;
        state[6] += g;
        state[7] += h;
    }

    return toHex(state);
}

bool hashEquals(const std::string &expectedHex, const std::string &actualHex)
{
    if (expectedHex.size() != actualHex.size())
    {
        return false;
    }
    for (size_t i = 0; i < expectedHex.size(); ++i)
    {
        char a = expectedHex[i];
        char b = actualHex[i];
        if (a >= 'A' && a <= 'F')
        {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'F')
        {
            b = static_cast<char>(b - 'A' + 'a');
        }
        if (a != b)
        {
            return false;
        }
    }
    return true;
}
