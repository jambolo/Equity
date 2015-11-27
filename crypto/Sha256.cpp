#include "Sha256.h"

#include <openssl/evp.h>

namespace Crypto
{

Sha256Hash sha256(std::vector<uint8_t> const & input)
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

    return Sha256Hash(output, output + outputLength);
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
    Checksum c = doubleSha256(input, length);
    c.resize(CHECKSUM_SIZE);
    return c;
}

} // namespace Crypto
