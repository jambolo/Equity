#include "Sha256.h"

#include <openssl/evp.h>
#include <cassert>
#include <algorithm>

namespace Crypto
{

Sha256Hash sha256(std::vector<uint8_t> const & input)
{
    return sha256(input.data(), input.size());
}

Sha256Hash sha256(Sha256Hash const & input)
{
    return sha256(input.data(), input.size());
}

Sha256Hash sha256(uint8_t const * input, size_t length)
{
    EVP_MD const * md = EVP_sha256();
    EVP_MD_CTX * mdctx = EVP_MD_CTX_create();

    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, length);
    EVP_DigestFinal_ex(mdctx, output, &outputLength);
    EVP_MD_CTX_destroy(mdctx);

    Sha256Hash hash;
    assert(outputLength == hash.size());
    std::copy(output, output + outputLength, hash.begin());

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
