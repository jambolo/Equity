#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const RIPEMD160_HASH_SIZE = 160 / 8;                         //!< The sized of a RIPEMD-160 hash in bytes
typedef std::array<uint8_t, RIPEMD160_HASH_SIZE> Ripemd160Hash;     //!< A RIPEMD-160 hash

//! Computes the RIPEMD-160 hash of the input
Ripemd160Hash ripemd160(std::vector<uint8_t> const & input);

//! Computes the RIPEMD-160 hash of the input
Ripemd160Hash ripemd160(uint8_t const * input, size_t length);

//! Computes the RIPEMD-160 hash of an std::array input
template <size_t N>
Ripemd160Hash ripemd160(std::array<uint8_t, N> const & input)
{
    return ripemd160(input.data(), input.size());
}

} // namespace Crypto
