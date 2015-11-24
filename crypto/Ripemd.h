#pragma once

#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const RIPEMD160_HASH_SIZE = 160 / 8;
typedef std::vector<uint8_t> Ripemd160Hash;

Ripemd160Hash ripemd160(std::vector<uint8_t> const & input);
Ripemd160Hash ripemd160(uint8_t const * input, size_t length);

} // namespace Crypto
