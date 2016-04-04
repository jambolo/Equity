#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

//! Functions that convert between binary and the Base58Check format. Typically used for addresses and private keys.
namespace Base58Check
{

//! Encode the input into Base58Check format
std::string encode(std::vector<uint8_t> const & input, unsigned version);

//! Encode the input into Base58Check format
std::string encode(uint8_t const * input, size_t length, unsigned version);

//! Encode the input into Base58Check format
template <size_t N>
std::string encode(std::array<uint8_t, N> const & input, unsigned version)
{
    return encode(input.data(), input.size(), version);
}

//! Decode the Base58Check format into binary
bool decode(std::string const & input, std::vector<uint8_t> & output, unsigned & version);

//! Decode the Base58Check format into binary
bool decode(char const * input, std::vector<uint8_t> & output, unsigned & version);

//! Decode the Base58Check format into binary
bool decode(char const * input, uint8_t * output, size_t size, unsigned & version);

} // namespace Base58Check
} // namespace Equity
