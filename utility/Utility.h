#pragma once

#include <crypto/Sha256.h>
#include <cstdint>
#include <string>
#include <vector>

namespace Utility
{

//! Converts a vector to a hex string
std::string toHex(std::vector<uint8_t> const & v);

//! Converts a vector to a hex string
std::string toHex(uint8_t const * v, size_t length);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHex(std::string const & x);

//! Converts a hex string to a vector
std::vector<uint8_t> fromHex(char const * x, size_t length);

//! Converts a hex JSON string value to a vector
std::vector<uint8_t> fromJson(std::string const & j);

//! Converts a generic vector to a JSON array value
template <typename T>
std::string toJson(std::vector<T> const & v)
{
    std::string json;
    json += '[';
    typename std::vector<T>::const_iterator i = v.begin();
    if (i != v.end())
    {
        json += i->toJson();
        ++i;
    }
    for (; i != v.end(); ++i)
    {
        json += ',';
        json += i->toJson();
    }
    json += ']';
    return json;
}

//! Converts a vector of bytes to a hex JSON string value
template <> std::string toJson<uint8_t>(std::vector<uint8_t> const & v);

// Shortens a string replacing the middle with ...
std::string shorten(std::string const & in, size_t size = 11);

} // namespace Utility
