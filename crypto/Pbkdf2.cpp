#include "Pbkdf2.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/pwdbased.h>
#include <wolfssl/wolfcrypt/sha512.h>

#include <cassert>
#include <cstdint>

namespace Crypto
{

std::vector<uint8_t> pbkdf2HmacSha512(uint8_t const * password, size_t passwordSize,
                                      uint8_t const * salt, size_t saltSize,
                                      int count,
                                      size_t size)
{
    assert(count > 0);
    assert(size > 0);

    int rc;
    std::vector<uint8_t> key(WC_SHA512_DIGEST_SIZE);

    rc = wc_PBKDF2(&key[0], password, (word32)passwordSize, salt, (word32)saltSize, count, WC_SHA512_DIGEST_SIZE, WC_SHA512);
    if (!rc)
        return std::vector<uint8_t>();

    return key;
}

} // namespace Crypto
