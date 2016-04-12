#pragma once

#include <cstdint>
#include <vector>
#include <array>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const SHA256_HASH_SIZE = 256 / 8;                    //!< Size of a SHA-256 hash in bytes
typedef std::array<uint8_t, SHA256_HASH_SIZE> Sha256Hash;   //!< A SHA-256 hash
typedef std::vector<Sha256Hash> Sha256HashList;             //!< A vector of SHA-256 hashes

size_t const CHECKSUM_SIZE = 4;                             //!< Size of a checksum in bytes
typedef std::array<uint8_t, CHECKSUM_SIZE> Checksum;        //!< A 4-byte checksum from the first 4 bytes of doubleSha256(input)

//! Computes the SHA-256 hash of the input
Sha256Hash sha256(std::vector<uint8_t> const & input);

// //! Computes the SHA-256 hash a SHA-256 hash
// Sha256Hash sha256(Sha256Hash const & input);

//! Computes the SHA-256 hash of the input
Sha256Hash sha256(uint8_t const * input, size_t length);

//! Computes the SHA-256 hash of an std::array input
template <size_t N>
Sha256Hash sha256(std::array<uint8_t, N> const & input)
{
    return sha256(input.data(), input.size());
}

//! Computes the double-SHA-256 hash of the input
Sha256Hash doubleSha256(std::vector<uint8_t> const & input);

//! Computes the double-SHA-256 hash of the input
Sha256Hash doubleSha256(uint8_t const * input, size_t length);

//! Computes the double-SHA-256 checksum of the input
Checksum checksum(std::vector<uint8_t> const & input);

//! Computes the double-SHA-256 checksum of the input
Checksum checksum(uint8_t const * input, size_t length);

} // namespace Crypto
