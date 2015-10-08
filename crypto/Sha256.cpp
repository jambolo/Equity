#include "Sha256.h"

#include <openssl/evp.h>

using namespace Crypto;

Sha256::Hash Sha256::operator ()(std::vector<uint8_t> const & input)
{
    return operator()(&input[0], input.size());
}

Sha256::Hash Sha256::operator ()(uint8_t const * input, size_t length)
{
    EVP_MD const * md = EVP_sha256();
    EVP_MD_CTX * mdctx = EVP_MD_CTX_create();

    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];

    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, length);
    EVP_DigestFinal_ex(mdctx, output, &outputLength);
    EVP_MD_CTX_destroy(mdctx);

    return Hash(output, output+outputLength);
}
