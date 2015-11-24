#pragma once

#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const SHA256_HASH_SIZE = 256 / 8;
typedef std::vector<uint8_t> Sha256Hash;
typedef std::vector<Sha256Hash> Sha256HashList;

Sha256Hash sha256(std::vector<uint8_t> const & input);
Sha256Hash sha256(uint8_t const * input, size_t length);

inline Sha256Hash doubleSha256(uint8_t const * input, size_t length)
{
    return sha256(sha256(input, length));
}

inline Sha256Hash doubleSha256(std::vector<uint8_t> const & input)
{
    return sha256(sha256(input.data(), input.size()));
}

} // namespace Crypto
