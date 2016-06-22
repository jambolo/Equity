#include "Pbkdf2.h"

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <cassert>

namespace Crypto
{

std::vector<uint8_t> pbkdf2HmacSha512(uint8_t const * password, size_t passwordSize,
                                      uint8_t const * salt, size_t saltSize,
                                      int count,
                                      size_t size)
{
    assert(count > 0);
    assert(size > 0);

    std::vector<uint8_t> key(size);
    EVP_MD const * md = EVP_sha512();
    int rv = PKCS5_PBKDF2_HMAC((char const *)password,
                               (int)passwordSize,
                               salt,
                               (int)saltSize,
                               count,
                               md,
                               (int)key.size(),
                               key.data());
    assert(rv == 1);
    return key;
}

} // namespace Crypto
