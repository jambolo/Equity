#include "PublicKey.h"

#include "PrivateKey.h"
#include "utility/Utility.h"

#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"
#include <cassert>
#include <memory>

using namespace Equity;

namespace
{

// TODO: This function should call the crypto library rather than using openssl directly here
bool isValidUncompressedPublicKey(uint8_t const * data, size_t size)
{
    return size == PublicKey::UNCOMPRESSED_SIZE && data[0] == 4;
}

// TODO: This function should call the crypto library rather than using openssl directly here
bool isValidCompressedPublicKey(uint8_t const * data, size_t size)
{
    return size == PublicKey::COMPRESSED_SIZE && (data[0] == 2 || data[0] == 3);
}

}

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : PublicKey(k.data(), k.size())
{
}

PublicKey::PublicKey(uint8_t const * data, size_t size)
    : valid_(true)
{
    if (!isValidUncompressedPublicKey(data, size) && !isValidCompressedPublicKey(data, size))
    {
        valid_ = false;
        return;
    }

    value_.assign(data, data + size);
    compressed_ = (data[0] != 4);
}

PublicKey::PublicKey(PrivateKey const & k)
    : valid_(false)
{
    // TODO: This function should call the crypto library rather than using openssl directly here

    std::shared_ptr<BIGNUM> prvKey(BN_new(), BN_free);
    BN_bin2bn(k.value().data(), (int)PrivateKey::SIZE, prvKey.get());

    std::shared_ptr<EC_GROUP> group(EC_GROUP_new_by_curve_name(NID_secp256k1), EC_GROUP_free);
    std::shared_ptr<EC_POINT> pubKey(EC_POINT_new(group.get()), EC_POINT_free);
    std::shared_ptr<BN_CTX>   ctx(BN_CTX_new(), BN_CTX_free);

    if (!EC_POINT_mul(group.get(), pubKey.get(), prvKey.get(), nullptr, nullptr, ctx.get()))
        return;

    value_.resize(UNCOMPRESSED_SIZE);
    size_t length = EC_POINT_point2oct(group.get(),
                                       pubKey.get(),
                                       POINT_CONVERSION_UNCOMPRESSED,
                                       value_.data(),
                                       value_.size(),
                                       ctx.get());
    assert(length == UNCOMPRESSED_SIZE);
    assert(value_[0] == 4);
    valid_ = (length == UNCOMPRESSED_SIZE) && (value_[0] == 4);
    compressed_ = false;
}

std::string PublicKey::toHex() const
{
    return Utility::toHex(value_);
}
