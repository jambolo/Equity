// Thin shim over the Rust `pbkdf2`+`hmac`+`sha2` crates (see libs/crypto/src/hash_ffi.rs).

#include "Pbkdf2.h"

#include <cstddef>
#include <cstdint>

extern "C" bool crypto_pbkdf2_hmac_sha512(uint8_t const * password,
                                          size_t          password_size,
                                          uint8_t const * salt,
                                          size_t          salt_size,
                                          int             count,
                                          uint8_t *       out,
                                          size_t          out_capacity);

std::vector<uint8_t> Crypto::pbkdf2HmacSha512(uint8_t const * password,
                                              size_t          passwordSize,
                                              uint8_t const * salt,
                                              size_t          saltSize,
                                              int             count,
                                              size_t          size)
{
    std::vector<uint8_t> out(size);
    if (!crypto_pbkdf2_hmac_sha512(password, passwordSize, salt, saltSize, count, out.data(), size))
    {
        return std::vector<uint8_t>();
    }
    return out;
}
