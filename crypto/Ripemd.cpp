#include "Ripemd.h"

#include <algorithm>
#include <cassert>
#include <openssl/evp.h>

namespace Crypto
{

Ripemd160Hash ripemd160(std::vector<uint8_t> const & input)
{
    return ripemd160(input.data(), input.size());
}

Ripemd160Hash ripemd160(uint8_t const * input, size_t length)
{
    EVP_MD const * md = EVP_ripemd160();
    EVP_MD_CTX * mdctx = EVP_MD_CTX_create();

    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, length);
    EVP_DigestFinal_ex(mdctx, output, &outputLength);
    EVP_MD_CTX_destroy(mdctx);

    Ripemd160Hash hash = {};
    assert(outputLength == hash.size());
    std::copy(output, output + outputLength, hash.begin());

    return hash;
}

} // namespace Crypto
