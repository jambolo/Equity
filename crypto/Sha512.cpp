#include "Sha512.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha512.h>

#include <algorithm>
#include <cassert>

namespace Crypto
{

Sha512Hash sha512(std::vector<uint8_t> const & input)
{
    return sha512(input.data(), input.size());
}

//     Sha512Hash sha512(Sha512Hash const & input)
//     {
//         return sha512(input.data(), input.size());
//     }

Sha512Hash sha512(uint8_t const * input, size_t length)
{
    Sha512Hash hash = {};
    return hash;
}

} // namespace Crypto
