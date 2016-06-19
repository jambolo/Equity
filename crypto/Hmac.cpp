#include "Hmac.h"

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <cassert>

namespace Crypto
{

Sha512Hash hmacSha512(uint8_t const * key, size_t keySize, uint8_t const * message, size_t messageSize)
{
    EVP_MD const * md = EVP_sha512();
        
    unsigned outputLength;
    uint8_t output[EVP_MAX_MD_SIZE];
        
    HMAC(md, key, (int)keySize, message, (int)messageSize, output, &outputLength);
        
    Sha512Hash hash = {};
    assert(outputLength == hash.size());
    std::copy(output, output + outputLength, hash.begin());
        
    return hash;

}
    
} // namespace Crypto
