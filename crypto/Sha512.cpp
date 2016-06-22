#include "Sha512.h"

#include <algorithm>
#include <cassert>
#include <openssl/evp.h>

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
    EVP_MD const * md = EVP_sha512();
    EVP_MD_CTX * mdctx = EVP_MD_CTX_create();

    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, length);
    EVP_DigestFinal_ex(mdctx, output, &outputLength);
    EVP_MD_CTX_destroy(mdctx);

    Sha512Hash hash = {};
    assert(outputLength == hash.size());
    std::copy(output, output + outputLength, hash.begin());

    return hash;
}

} // namespace Crypto
