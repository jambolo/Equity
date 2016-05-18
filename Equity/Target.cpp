#include "Target.h"

#include "crypto/Sha256.h"

#include <algorithm>
#include <cmath>
#include <cassert>

using namespace Equity;

namespace
{
    uint32_t const MANTISSA_MASK = 0x7fffff;
    int const EXPONENT_OFFSET = 24;

    uint32_t extractMantissa(uint32_t x)
    {
        return x & MANTISSA_MASK;
    }

    int extractExponent(uint32_t x)
    {
        return ((x >> EXPONENT_OFFSET) & 0xff) - 3;
    }
}

Crypto::Sha256Hash const Target::DIFFICULTY_1 =
{
    {
        0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    }
};

Target::Target(Crypto::Sha256Hash const & hash)
    : hash_(hash)
    , compact_(convertToCompact(hash))
{
}

Target::Target(uint32_t compact)
    : hash_(convertToHash(compact))
{
    compact_ = std::max(compact, TARGET_0_COMPACT);
}

double Target::difficulty() const
{
    return toDouble(DIFFICULTY_1_COMPACT) / toDouble(compact_);
}

Crypto::Sha256Hash Target::convertToHash(uint32_t compact)
{
    assert((compact & 0x800000) == 0);         // No negative numbers

    uint32_t mantissa = extractMantissa(compact);
    int exponent = extractExponent(compact);

    assert(exponent >= -3 && exponent <= 0x1d);

    Crypto::Sha256Hash out;
    out.fill(0);
    if (exponent >= 0)
    {
        out[Crypto::SHA256_HASH_SIZE - 3 - exponent] = (mantissa >> 16) & 0xff;
        out[Crypto::SHA256_HASH_SIZE - 2 - exponent] = (mantissa >>  8) & 0xff;
        out[Crypto::SHA256_HASH_SIZE - 1 - exponent] =         mantissa & 0xff;
    }
    else
    {
        if (exponent >= -2)
            out[Crypto::SHA256_HASH_SIZE - 3 - exponent] = (mantissa >> 16) & 0xff;
        if (exponent >= -1)
            out[Crypto::SHA256_HASH_SIZE - 2 - exponent] = (mantissa >> 8) & 0xff;
    }

    return out;
}

uint32_t Target::convertToCompact(Crypto::Sha256Hash const & hash)
{
    // Count the number of 0 bytes
    Crypto::Sha256Hash::const_iterator i = std::find_if_not(hash.begin(), hash.end(), [](uint8_t x) {
        return x == 0;
    });
    int zeros = (int)std::distance(hash.begin(), i);

    if (zeros == Crypto::SHA256_HASH_SIZE)
        return TARGET_0_COMPACT;

    // Get the exponent
    int exponent = (int)Crypto::SHA256_HASH_SIZE - 3 - zeros;

    // Get the mantissa
    uint32_t m0;
    uint32_t m1;
    uint32_t m2;
    if (exponent >= 0)
    {
        m0 = hash[Crypto::SHA256_HASH_SIZE - 1 - exponent];
        m1 = hash[Crypto::SHA256_HASH_SIZE - 2 - exponent];
        m2 = hash[Crypto::SHA256_HASH_SIZE - 3 - exponent];
    }
    else
    {
        m0 = 0;
        m1 = (exponent >= -1) ? hash[Crypto::SHA256_HASH_SIZE - 2 - exponent] : 0;
        m2 = (exponent >= -2) ? hash[Crypto::SHA256_HASH_SIZE - 3 - exponent] : 0;
    }
    uint32_t mantissa = (m2 << 16) | (m1 << 8) | m0;

    // Mantissa is signed, so adjust if mantissa >= 0x800000
    if (mantissa >= 0x800000)
    {
        mantissa >>= 8;
        exponent += 1;
    }
    uint32_t compact = (exponent + 3) << EXPONENT_OFFSET | mantissa;

    return compact;
}

double Target::toDouble(uint32_t compact)
{
    int mantissa = extractMantissa(compact);
    int exponent = extractExponent(compact);
    return double(mantissa) * pow(256.0, exponent);
}
