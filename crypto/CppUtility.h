#pragma once

#if !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
#define CRYPTO_CPPUTILITY_H_INCLUDED

#include <memory>
#include <array>
#include <openssl/bn.h>
#include <openssl/ec.h>
#include <openssl/evp.h>

#define DECLARE_AUTO_POINTER(TYPE, FREE)                    \
struct TYPE##_deleter                                       \
{                                                           \
    void operator ()(TYPE * p) { FREE(p); }                 \
};                                                          \
typedef std::unique_ptr<TYPE, TYPE##_deleter> auto_##TYPE

namespace Crypto
{
    DECLARE_AUTO_POINTER(BIGNUM, BN_free);
    DECLARE_AUTO_POINTER(BN_CTX, BN_CTX_free);
    DECLARE_AUTO_POINTER(EC_GROUP, EC_GROUP_free);
    DECLARE_AUTO_POINTER(EC_KEY, EC_KEY_free);
    DECLARE_AUTO_POINTER(EC_POINT, EC_POINT_free);
    DECLARE_AUTO_POINTER(EVP_MD_CTX, EVP_MD_CTX_destroy);
} // namespace Crypto


#endif // !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
