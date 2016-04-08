#include "Ecdsa.h"

#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"

#include <memory>

typedef std::unique_ptr<BIGNUM, void(BIGNUM *)> auto_BIGNUM_ptr;
typedef std::unique_ptr<EC_GROUP, void(EC_GROUP *)> auto_EC_GROUP_ptr;
typedef std::unique_ptr<EC_POINT, void(EC_POINT *)> auto_EC_POINT_ptr;
typedef std::unique_ptr<BN_CTX, void(BN_CTX *)> auto_BN_CTX_ptr;

namespace Crypto
{


    bool Ecdsa::derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey)
    {
//        auto_BIGNUM_ptr prvKey_bn(BN_new(), BN_free);
//        BN_bin2bn(prvKey.data(), (int)prvKey.size(), prvKey_bn.get());
//
//        auto_EC_GROUP_ptr group(EC_GROUP_new_by_curve_name(NID_secp256k1), EC_GROUP_free);
//        auto_EC_POINT_ptr point(EC_POINT_new(group.get()), EC_POINT_free);
//        auto_BN_CTX_ptr   ctx(BN_CTX_new(), BN_CTX_free);
//
//        if (!EC_POINT_mul(group.get(), point.get(), prvKey_bn.get(), NULL, NULL, ctx.get()))
//            return false;
//
//        std::fill(pubKey.begin(), pubKey.end(), 0);
//        size_t length = EC_POINT_point2oct(group.get(),
//            point.get(),
//            POINT_CONVERSION_UNCOMPRESSED,
//            pubKey.data(),
//            pubKey.size(),
//            ctx.get());
//        std::rotate(pubKey.begin(), pubKey.begin() + length, pubKey.end());

        return true;

    }


} // namespace Crypto
