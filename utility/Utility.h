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
template<size_t N>
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

//! Converts a vector to a hex string
std::string toHexR(std::vector<uint8_t> const & v);

//! Converts a vector to a hex string
template<size_t N>
std::string toHexR(std::array<uint8_t, N> const & a)
{
    return toHexR(a.data(), a.size());
}

//! Converts a vector to a hex string
std::string toHexR(uint8_t const * v, size_t length);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHexR(std::string const & x);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHexR(char const * x, size_t length);

//! Converts a hex JSON string value to a vector
std::vector<uint8_t> fromJson(std::string const & j);

//! Converts a generic vector to a JSON array value
template <typename T>
std::string toJson(std::vector<T> const & v)
{
    return toJson(v.data(), v.size());
}

//! Converts a generic std::array to a JSON array value
template <typename T, size_t N>
std::string toJson(std::array<T, N> const & a)
{
    return toJson(a.data(), a.size());
}

//! Converts a generic array to a JSON array value
template <typename T>
std::string toJson(T const * values, size_t size)
{
    std::string json;
    json += '[';
    if (size > 0)
    {
        json += values[0].toJson();
        for (size_t i = 1; i < size; ++i)
        {
            json += ',';
            json += values[i].toJson();
        }
    }
    json += ']';
    return json;
}

//! Converts a vector of bytes to a hex JSON string value
template <> std::string toJson<uint8_t>(uint8_t const * a, size_t size);

// Shortens a string replacing the middle with ...
std::string shorten(std::string const & in, size_t size = 11);

} // namespace Utility
