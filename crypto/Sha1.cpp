#include "Sha1.h"

#include <openssl/evp.h>
#include <cassert>
#include <algorithm>

namespace Crypto
{

Sha1Hash sha1(std::vector<uint8_t> const & input)
{
    return sha1(input.data(), input.size());
}

Sha1Hash sha1(uint8_t const * input, size_t length)
{
    EVP_MD const * md = EVP_sha1();
    EVP_MD_CTX * mdctx = EVP_MD_CTX_create();

    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, length);
    EVP_DigestFinal_ex(mdctx, output, &outputLength);
    EVP_MD_CTX_destroy(mdctx);

    Sha1Hash hash = {};
    assert(outputLength == hash.size());
    std::copy(output, output + outputLength, hash.begin());

    return hash;
}

} // namespace Crypto
