#include "Sha256.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sha256.h>

#include <algorithm>
#include <cassert>

namespace Crypto
{

Sha256Hash sha256(std::vector<uint8_t> const & input)
{
    return sha256(input.data(), input.size());
}

// Sha256Hash sha256(Sha256Hash const & input)
// {
//     return sha256(input.data(), input.size());
// }

Sha256Hash sha256(uint8_t const * input, size_t length)
{
    Sha256Hash hash = {};
    
    wc_Sha256 sha;
    wc_InitSha256(&sha);
    wc_Sha256Update(&sha, input, static_cast<word32>(length));
    wc_Sha256Final(&sha, hash.data());
    
    return hash;
}

Sha256Hash doubleSha256(std::vector<uint8_t> const & input)
{
    return sha256(sha256(input.data(), input.size()).data(), SHA256_HASH_SIZE);
}

Sha256Hash doubleSha256(uint8_t const * input, size_t length)
{
    return sha256(sha256(input, length).data(), SHA256_HASH_SIZE);
}

Checksum checksum(std::vector<uint8_t> const & input)
{
    return checksum(input.data(), input.size());
}

Checksum checksum(uint8_t const * input, size_t length)
{
    Checksum c;
    Sha256Hash hash = doubleSha256(input, length);
    std::copy(hash.begin(), hash.begin() + CHECKSUM_SIZE, c.begin());
    return c;
}

} // namespace Crypto
