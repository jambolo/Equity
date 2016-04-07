#include "PublicKey.h"

#include "PrivateKey.h"
#include "utility/Utility.h"

#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"
#include <cassert>
#include <memory>

using namespace Equity;

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : valid_(k.size() == SIZE && k[0] == 4)
{
    if (valid_)
        std::copy(k.begin(), k.end(), value_.begin());
}

PublicKey::PublicKey(uint8_t const * k)
    : valid_(k[0] == 4)
{
    if (valid_)
        std::copy(k, k + SIZE, value_.begin());
}

PublicKey::PublicKey(PrivateKey const & k)
    : valid_(false)
{
    std::shared_ptr<BIGNUM> prvKey(BN_new(), BN_free);
    BN_bin2bn(k.value().data(), (int)PrivateKey::SIZE, prvKey.get());

    std::shared_ptr<EC_GROUP> group(EC_GROUP_new_by_curve_name(NID_secp256k1), EC_GROUP_free);
    std::shared_ptr<EC_POINT> pubKey(EC_POINT_new(group.get()), EC_POINT_free);
    std::shared_ptr<BN_CTX>   ctx(BN_CTX_new(), BN_CTX_free);

    if (!EC_POINT_mul(group.get(), pubKey.get(), prvKey.get(), NULL, NULL, ctx.get()))
        return;

    size_t length = EC_POINT_point2oct(group.get(),
                                       pubKey.get(),
                                       POINT_CONVERSION_UNCOMPRESSED,
                                       value_.data(),
                                       value_.size(),
                                       ctx.get());
    assert(length == SIZE);
    assert(value_[0] == 4);
    valid_ = (length == SIZE) && (value_[0] == 4);
}

std::string PublicKey::toHex() const
{
    return Utility::toHex(value_);
}
