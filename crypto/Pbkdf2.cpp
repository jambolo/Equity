#include "Pbkdf2.h"

#include <openssl/evp.h>
#include <openssl/sha.h>

#include <cassert>

namespace Crypto
{

std::vector<uint8_t> pbkdf2HmacSha512(std::vector<uint8_t> const & password,
                                      std::vector<uint8_t> const & salt,
                                      int                          count,
                                      size_t                       size)
{
    assert(count > 0);
    assert(size > 0);

    std::vector<uint8_t> key(size);
    EVP_MD const * md = EVP_sha512();
    int rv = PKCS5_PBKDF2_HMAC((char const *)password.data(),
                               (int)password.size(),
                               salt.data(),
                               (int)salt.size(),
                               count,
                               md,
                               (int)key.size(),
                               key.data());
    assert(rv == 0);
    return key;
}

} // namespace Crypto
