#include "PublicKey.h"

#include "PrivateKey.h"
#include "Utility.h"

#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"
#include <memory>

using namespace Equity;

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : value_(k)
    , valid_(k.size() == SIZE)
{
}

PublicKey::PublicKey(uint8_t const * k)
    : value_(k, k + SIZE)
    , valid_(true)
{
}

PublicKey::PublicKey(PrivateKey const & k)
    : value_(SIZE)
    , valid_(false)
{
    std::shared_ptr<EC_KEY> keyPair(EC_KEY_new_by_curve_name(NID_secp256k1), EC_KEY_free);
    if (!keyPair)
    {
        return;
    }

    std::shared_ptr<BIGNUM> prvKeyBn(BN_new(), BN_free);
    BN_bin2bn(&k.value()[0], (int)PrivateKey::SIZE, prvKeyBn.get());

    if (!EC_KEY_set_private_key(keyPair.get(), prvKeyBn.get()))
    {
        return;
    }

    if (!EC_KEY_check_key(keyPair.get()))
    {
        return;
    }

    
    std::shared_ptr<BN_CTX> ctx(BN_CTX_new(), BN_CTX_free);
    EC_POINT const *        pubKey = EC_KEY_get0_public_key(keyPair.get());
    EC_GROUP const *        group = EC_KEY_get0_group(keyPair.get());

    size_t length = EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_UNCOMPRESSED, NULL, 0, ctx.get());

    if (EC_POINT_point2oct(group, pubKey, POINT_CONVERSION_UNCOMPRESSED, &value_[SIZE-length], length, ctx.get()) != length)
    {
        return;
    }

    valid_ = true;
}

std::string PublicKey::toHex() const
{
    return Utility::vtox(value_);
}

