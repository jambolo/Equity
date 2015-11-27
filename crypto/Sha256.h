#pragma once

#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Hashes are always big-endian

size_t const SHA256_HASH_SIZE = 256 / 8;            //!< Size of a SHA-256 hash in bytes
typedef std::vector<uint8_t> Sha256Hash;
typedef std::vector<Sha256Hash> Sha256HashList;

size_t const CHECKSUM_SIZE = 4;                     //!< Size of a double-SHA-256 checksum in bytes
typedef std::vector<uint8_t> Checksum;

//! Computes the SHA-256 hash of the input
Sha256Hash sha256(std::vector<uint8_t> const & input);

//! Computes the SHA-256 hash of the input
Sha256Hash sha256(uint8_t const * input, size_t length);

//! Computes the double-SHA-256 hash of the input
Sha256Hash doubleSha256(std::vector<uint8_t> const & input);

//! Computes the double-SHA-256 hash of the input
Sha256Hash doubleSha256(uint8_t const * input, size_t length);

//! Computes the double-SHA-256 checksum of the input
Checksum checksum(std::vector<uint8_t> const & input);

//! Computes the double-SHA-256 checksum of the input
Checksum checksum(uint8_t const * input, size_t length);

} // namespace Crypto
