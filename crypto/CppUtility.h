#pragma once

#if !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
#define CRYPTO_CPPUTILITY_H_INCLUDED

#include <memory>
#include <array>
#include <openssl/bn.h>
#include <openssl/ec.h>

namespace Crypto
{
    struct BIGNUM_deleter
    {
        void operator ()(BIGNUM * p) { BN_free(p); }
    };
    typedef std::unique_ptr<BIGNUM, BIGNUM_deleter> auto_BIGNUM;
    
    struct EC_GROUP_deleter
    {
        void operator ()(EC_GROUP * p) { EC_GROUP_free(p); }
    };
    typedef std::unique_ptr<EC_GROUP, EC_GROUP_deleter> auto_EC_GROUP;
    
    struct EC_POINT_deleter
    {
        void operator ()(EC_POINT * p) { EC_POINT_free(p); }
    };
    typedef std::unique_ptr<EC_POINT, EC_POINT_deleter> auto_EC_POINT;
    
    struct BN_CTX_deleter
    {
        void operator ()(BN_CTX * p) { BN_CTX_free(p); }
    };
    typedef std::unique_ptr<BN_CTX, BN_CTX_deleter> auto_BN_CTX;

    
} // namespace Crypto


#endif // !defined(CRYPTO_CPPUTILITY_H_INCLUDED)
