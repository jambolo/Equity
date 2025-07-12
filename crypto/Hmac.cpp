#include "Hmac.h"
#include "Sha512.h"

#include "utility/Debug.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/hmac.h>
#include <wolfssl/wolfcrypt/sha512.h>

#include <cassert>

namespace Crypto
{

Sha512Hash hmacSha512(uint8_t const * key, size_t keySize, uint8_t const * message, size_t messageSize)
{
    NOT_YET_IMPLEMENTED();
    Sha512Hash hash = {};
    return hash;
}

} // namespace Crypto
