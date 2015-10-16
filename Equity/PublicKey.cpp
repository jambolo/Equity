#include "PublicKey.h"

#include "PrivateKey.h"
#include "utility/Utility.h"

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
    std::shared_ptr<BIGNUM> prvKey(BN_new(), BN_free);
    BN_bin2bn(&k.value()[0], (int)PrivateKey::SIZE, prvKey.get());

    std::shared_ptr<EC_GROUP> group(EC_GROUP_new_by_curve_name(NID_secp256k1), EC_GROUP_free);
    std::shared_ptr<EC_POINT> pubKey(EC_POINT_new(group.get()), EC_POINT_free);
    std::shared_ptr<BN_CTX>   ctx(BN_CTX_new(), BN_CTX_free);

    if (!EC_POINT_mul(group.get(), pubKey.get(), prvKey.get(), NULL, NULL, ctx.get()))
    {
        return;
    }

    size_t length = EC_POINT_point2oct(group.get(), pubKey.get(), POINT_CONVERSION_UNCOMPRESSED, NULL, 0, ctx.get());

    if (EC_POINT_point2oct(group.get(), pubKey.get(), POINT_CONVERSION_UNCOMPRESSED, &value_[SIZE-length], length, ctx.get()) != length)
    {
        return;
    }

    valid_ = true;
}

std::string PublicKey::toHex() const
{
    return Utility::vtox(value_);
}

