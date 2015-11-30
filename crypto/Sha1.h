#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const SHA1_HASH_SIZE = 160 / 8;                  //!< Size of a SHA-1 HASH in bytes
typedef std::array<uint8_t, SHA1_HASH_SIZE> Sha1Hash;   //!< A SHA-1 hash

//! Returns the SHA-1 hash of the input
Sha1Hash sha1(std::vector<uint8_t> const & input);

//! Returns the SHA-1 hash of the input
Sha1Hash sha1(uint8_t const * input, size_t length);

//! Returns the SHA-1 hash of the input
template<size_t N>
Sha1Hash sha1(std::array<uint8_t, N> const & input)
{
    return sha1(input.data(), input.size());
}

} // namespace Crypto
