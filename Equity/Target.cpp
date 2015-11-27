#include "Target.h"

#include "crypto/Sha256.h"

#include <algorithm>
#include <cmath>

using namespace Equity;

Crypto::Sha256Hash const Target::DIFFICULTY_1 =
{
    0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

Target::Target(Crypto::Sha256Hash const & hash)
    : hash_(hash)
    , compact_(convertToCompact(hash))
{
}

Target::Target(uint32_t compact)
    : hash_(convertToHash(compact))
{
    compact_ = std::max(compact, TARGET_0_COMPACT)
}

float Target::difficulty() const
{
    return toDouble(DIFFICULTY_1_COMPACT) / toDouble(compact_);
}

Crypto::Sha256Hash Target::convertToHash(uint32_t compact)
{
    assert((compact & 0x800000) == 0);         // No negative numbers

    uint32_t mantissa = compact & 0x7fffff;
    int exponent = ((compact >> 24) & 0xff) - 3;

    assert(exponent >= -3 && exponent <= 0x1d);

    Crypto::Sha256Hash out(0, Crypto::SHA256_HASH_SIZE);
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
    int zeros = std::distance(hash.begin(), i);

    if (zeros == Crypto::SHA256_HASH_SIZE)
        return TARGET_0_COMPACT;

    // Get the exponent
    int exponent = Crypto::SHA256_HASH_SIZE - 3 - zeros;

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
    uint32_t compact = (exponent + 3) << 24 | mantissa;

    return compact;
}

double Target::toDouble(uint32_t compact)
{
    int mantissa = compact & 0x7fffff;
    int exponent = ((compact >> 24) & 0xff) - 3;
    return double(mantissa) * pow(256.0, exponent);
}
