#include "PublicKey.h"

#include "Utility.h"

#include "openssl/ec.h"
#include "openssl/evp.h"
#include "openssl/obj_mac.h"

using namespace Equity;

template<typename T>
class AutoResource
{
public:

    AutoResource(T * resource, void (*free)(T*)) : resource_(resource), free_(free) {}
    ~AutoResource()
    {
        free_(resource_);
    }

    operator T * ()             { return resource_; }
    operator T const * () const { return resource_; }
    operator bool () const      { return resource_ != NULL; }

private:

    T * resource_;
    void (*free_)(T*);
};

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : value_(k)
    , valid_(k.size() == SIZE)
{
}

PublicKey::PublicKey(uint8_t const * k)
    : value_(k, k + SIZE)
{
    valid_ = true;
}

PublicKey::PublicKey(PrivateKey const & k)
{
    valid_ = false;

    AutoResource<EC_KEY> keyPair(EC_KEY_new_by_curve_name(NID_secp256k1), EC_KEY_free);
    if (keyPair)
    {
        return;
    }

    AutoResource<BIGNUM> prvKeyBn(BN_new(), BN_free);
    BN_bin2bn(&k.value()[0], (int)PrivateKey::SIZE, prvKeyBn);

    if (!EC_KEY_set_private_key(keyPair, prvKeyBn))
    {
        return;
    }

    if (!EC_KEY_check_key(keyPair))
    {
        return;
    }

    EC_POINT const * pubKey = EC_KEY_get0_public_key(keyPair);
//    size_t EC_POINT_point2oct(const EC_GROUP *group, pubKey, POINT_CONVERSION_UNCOMPRESSED,
//        unsigned char *buf, size_t len, BN_CTX *ctx);
//    AutoResource<BIGNUM> pubKeyBn(EC_POINT_point2bn(const EC_GROUP *, pubKey, point_conversion_form_t form, BIGNUM *, BN_CTX *), BN_free);

    valid_ = true;
}

std::string PublicKey::toHex() const
{
    return Utility::vtox(value_);
}

