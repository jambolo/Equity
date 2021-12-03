#pragma once

#include <crypto/Sha256.h>
#include <cstdint>
#include <string>
#include <vector>

//! @defgroup UtilityGroup Utility functions
//!
//! Various unorganized utility functions.

//! Various unorganized utility functions.
namespace Utility
{
//! @addtogroup UtilityGroup
//!@{

//! Converts a vector of bytes to a hex string
//!
//! @param  v   vector to convert
//!
//! @return     hex string
std::string toHex(std::vector<uint8_t> const & v);

//! Converts data to a hex string
//!
//! @param  v       data to convert
//! @param  length  length of data
//!
//! @return     hex string
std::string toHex(uint8_t const * v, size_t length);

//! Converts a std::array of bytes to a hex string
//!
//! @param  a       array to convert
//!
//! @return     hex string
template <size_t N>
std::string toHex(std::array<uint8_t, N> const & a)
{
    return toHex(a.data(), a.size());
}

//! Converts a hex string to a vector of bytes
//!
//! @param  x   hex string to convert
//!
//! @return     vector of bytes
std::vector<uint8_t> fromHex(std::string const & x);

//! Converts an unterminated hex string to a vector
//!
//! @param  x       hex string to convert
//! @param  length  length of the string
//!
//! @return     vector of bytes
std::vector<uint8_t> fromHex(char const * x, size_t length);

//! Converts a vector a bytes to a hex string, reversing the order of the bytes
//!
//! @param  v   vector to convert
//!
//! @return     hex string
//! @note   This is typically used for displaying values (typically 256-bit hashes) in the byte order used by Bitcoin.
std::string toHexR(std::vector<uint8_t> const & v);

//! Converts data to a hex string, reversing the order of the bytes
//!
//! @param  v       data to convert
//! @param  length  length of data
//!
//! @return     hex string
//! @note   This is typically used for displaying values (typically 256-bit hashes) in the byte order used by Bitcoin.
std::string toHexR(uint8_t const * v, size_t length);

//! Converts an array of bytes to a hex string, reversing the order of the bytes.
//!
//! @param  a   array to convert
//!
//! @return     hex string
//! @note   This is typically used for displaying values (typically 256-bit hashes) in the byte order used by Bitcoin.
template <size_t N>
std::string toHexR(std::array<uint8_t, N> const & a)
{
    return toHexR(a.data(), a.size());
}

//! Converts a hex string to a vector, reversing the order of the bytes
//!
//! @param  x       hex string to convert
//!
//! @return     vector of bytes
//! @note   This is typically used for importing values (typically 256-bit hashes) in the byte order used by Bitcoin.
std::vector<uint8_t> fromHexR(std::string const & x);

//! Converts an unterminated hex string to a vector, reversing the order of the bytes
//!
//! @param  x       hex string to convert
//! @param  length  length of the string
//!
//! @return     vector of bytes
//! @note   This is typically used for inmporting values (typically 256-bit hashes) in the byte order used by Bitcoin.
std::vector<uint8_t> fromHexR(char const * x, size_t length);

//! Shortens a string replacing the middle with ...
//!
//! @param  in      string to shorten
//! @param  size    maximum size of the resulting string
//!
//! @return     shortened string
std::string shorten(std::string const & in, size_t size = 11);

//!@}
} // namespace Utility
