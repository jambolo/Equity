// Thin shim over the Rust `sha2` crate (see libs/crypto/src/hash_ffi.rs).

#include "Sha512.h"

#include <cstddef>
#include <cstdint>

extern "C" void crypto_sha512(uint8_t const * input, size_t length, uint8_t * out);

namespace Crypto
{

Sha512Hash sha512(std::vector<uint8_t> const & input)
{
    return sha512(input.data(), input.size());
}

Sha512Hash sha512(uint8_t const * input, size_t length)
{
    Sha512Hash out{};
    crypto_sha512(input, length, out.data());
    return out;
}

} // namespace Crypto
