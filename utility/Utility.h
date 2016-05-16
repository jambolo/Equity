#pragma once

#include <crypto/Sha256.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Utility
{

//! Converts a vector to a hex string
std::string toHex(std::vector<uint8_t> const & v);

//! Converts an std::array to a hex string
template <size_t N>
std::string toHex(std::array<uint8_t, N> const & a)
{
    return toHex(a.data(), a.size());
}

//! Converts a vector to a hex string
std::string toHex(uint8_t const * v, size_t length);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHex(std::string const & x);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHex(char const * x, size_t length);

//! Converts a vector to a hex string, reversing the order of the bytes
std::string toHexR(std::vector<uint8_t> const & v);

//! Converts a vector to a hex string, reversing the order of the bytes
template <size_t N>
std::string toHexR(std::array<uint8_t, N> const & a)
{
    return toHexR(a.data(), a.size());
}

//! Converts a vector to a hex string, reversing the order of the bytes
std::string toHexR(uint8_t const * v, size_t length);

//! Converts a hex string to a vector, reversing the order of the bytes
std::vector<uint8_t> fromHexR(std::string const & x);

//! Converts a hex string to a vector, reversing the order of the bytes
std::vector<uint8_t> fromHexR(char const * x, size_t length);

// Shortens a string replacing the middle with ...
std::string shorten(std::string const & in, size_t size = 11);

} // namespace Utility
