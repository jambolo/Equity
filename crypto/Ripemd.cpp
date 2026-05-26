// Thin shim over the Rust `ripemd` crate (see libs/crypto/src/hash_ffi.rs).

#include "Ripemd.h"

#include <cstddef>
#include <cstdint>

extern "C" void crypto_ripemd160(uint8_t const * input, size_t length, uint8_t * out);

namespace Crypto
{

Ripemd160Hash ripemd160(std::vector<uint8_t> const & input)
{
    return ripemd160(input.data(), input.size());
}

Ripemd160Hash ripemd160(uint8_t const * input, size_t length)
{
    Ripemd160Hash out{};
    crypto_ripemd160(input, length, out.data());
    return out;
}

} // namespace Crypto
