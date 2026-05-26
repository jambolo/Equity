// Thin shim over the Rust `sha1` crate (see libs/crypto/src/hash_ffi.rs).

#include "Sha1.h"

#include <cstddef>
#include <cstdint>

extern "C" void crypto_sha1(uint8_t const * input, size_t length, uint8_t * out);

namespace Crypto
{

Sha1Hash sha1(std::vector<uint8_t> const & input)
{
    return sha1(input.data(), input.size());
}

Sha1Hash sha1(uint8_t const * input, size_t length)
{
    Sha1Hash out{};
    crypto_sha1(input, length, out.data());
    return out;
}

} // namespace Crypto
