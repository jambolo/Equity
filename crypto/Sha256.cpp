// Thin shim over the Rust `sha2` crate (see libs/crypto/src/hash_ffi.rs).

#include "Sha256.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>

extern "C" void crypto_sha256(uint8_t const * input, size_t length, uint8_t * out);
extern "C" void crypto_double_sha256(uint8_t const * input, size_t length, uint8_t * out);
extern "C" void crypto_checksum(uint8_t const * input, size_t length, uint8_t * out);

namespace Crypto
{

Sha256Hash sha256(std::vector<uint8_t> const & input)
{
    return sha256(input.data(), input.size());
}

Sha256Hash sha256(uint8_t const * input, size_t length)
{
    Sha256Hash out{};
    crypto_sha256(input, length, out.data());
    return out;
}

Sha256Hash doubleSha256(std::vector<uint8_t> const & input)
{
    return doubleSha256(input.data(), input.size());
}

Sha256Hash doubleSha256(uint8_t const * input, size_t length)
{
    Sha256Hash out{};
    crypto_double_sha256(input, length, out.data());
    return out;
}

Checksum checksum(std::vector<uint8_t> const & input)
{
    return checksum(input.data(), input.size());
}

Checksum checksum(uint8_t const * input, size_t length)
{
    Checksum out{};
    crypto_checksum(input, length, out.data());
    return out;
}

} // namespace Crypto
