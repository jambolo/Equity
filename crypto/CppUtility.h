#pragma once

#if !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
#define CRYPTO_CPPUTILITY_H_INCLUDED

#include <memory>
#include <array>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

namespace Crypto
{
    
#define DECLARE_UNIQUE_POINTER_TYPE(TYPE, FREE)             \
struct TYPE##_deleter                                       \
{                                                           \
    void operator ()(TYPE * p) { FREE(p); }                 \
};                                                          \
typedef std::unique_ptr<TYPE, TYPE##_deleter> auto_##TYPE
    
    DECLARE_UNIQUE_POINTER_TYPE(BIGNUM, BN_free);
    DECLARE_UNIQUE_POINTER_TYPE(BN_CTX, BN_CTX_free);
    DECLARE_UNIQUE_POINTER_TYPE(EC_GROUP, EC_GROUP_free);
    DECLARE_UNIQUE_POINTER_TYPE(EC_KEY, EC_KEY_free);
    DECLARE_UNIQUE_POINTER_TYPE(EVP_PKEY, EVP_PKEY_free);
    DECLARE_UNIQUE_POINTER_TYPE(EC_POINT, EC_POINT_free);
    DECLARE_UNIQUE_POINTER_TYPE(EVP_MD_CTX, EVP_MD_CTX_destroy);
    
} // namespace Crypto


#endif // !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
