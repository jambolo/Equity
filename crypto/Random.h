#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace Crypto
{

namespace Random
{

//! Returns true if the RNG has enough entropy
bool status();

//! Fills the buffer with random bytes.
//! @param[out]     buffer  buffer to hold bytes
//! @param          size    number of byte to generate
void getBytes(uint8_t * buffer, size_t size);

//! Adds entropy to the random byte generator.
//! @param      entropy     entropy to add
//! @param      size        number of bytes of entropy
void addEntropy(uint8_t const * buffer, size_t size, double entropy);

} // namespace Random
} // namespace Crypto
