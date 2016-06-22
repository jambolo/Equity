#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const SHA512_HASH_SIZE = 512 / 8;                        //!< Size of a SHA-512 hash in bytes
typedef std::array<uint8_t, SHA512_HASH_SIZE> Sha512Hash;       //!< A SHA-512 hash
typedef std::vector<Sha512Hash> Sha512HashList;                 //!< A vector of SHA-512 hashes

//! Computes the SHA-512 hash of the input.
//! @param  input   data to hash
Sha512Hash sha512(std::vector<uint8_t> const & input);

//! Computes the SHA-512 hash of the input.
//! @param  input   data to hash
//! @param  length  length of the data
Sha512Hash sha512(uint8_t const * input, size_t length);

//! Computes the SHA-512 hash of an std::array of uint8_t.
//! @param  input   data to hash
template <size_t N>
Sha512Hash sha512(std::array<uint8_t, N> const & input)
{
    return sha512(input.data(), input.size());
}

} // namespace Crypto
