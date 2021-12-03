#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{
//! @addtogroup EquityGroup
//!@{

//! Base58 format encoding and decoding.
namespace Base58
{
//! Encodes binary data into Base58 format.
//!
//! @param  input   binary data to encode
//!
//! @return         Base58 encoded string
std::string encode(std::vector<uint8_t> const & input);

//! Encodes binary data into Base58 format.
//!
//! @param  input   binary data to encode
//! @param  length  length of the binary data
//!
//! @return         Base58 encoded string
std::string encode(uint8_t const * input, size_t length);

//! Decodes Base58-encoded data into binary.
//!
//! @param      input   Base58 encoded string
//! @param[out] output  decoded binary data
//!
//! @return     true if the input was successfully decoded
bool decode(std::string const & input, std::vector<uint8_t> & output);

//! Decodes Base58-encoded data into binary.
//!
//! @param      input   Base58 encoded string (must be 0-terminated)
//! @param[out] output  decoded binary data
//!
//! @return     true if the input was successfully decoded
bool decode(char const * input, std::vector<uint8_t> & output);
}

//!@}
} // namespace Equity
