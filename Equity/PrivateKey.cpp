#include "PrivateKey.h"

#include "Base58Check.h"
#include "Utility.h"

#include <openssl\bn.h>

using namespace Equity;

static uint8_t const MAX_PRIVATE_KEY[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
};
static size_t const MAX_PRIVATE_KEY_SIZE = sizeof(MAX_PRIVATE_KEY);

PrivateKey::PrivateKey(std::string const & wif)
{
    valid_ = Base58Check::decode(wif, value_, version_) && isValid();
}

PrivateKey::PrivateKey(std::vector<uint8_t> const & k, unsigned v)
    : value_(k)
    , version_(v)
{
    valid_ = isValid();
}

PrivateKey::PrivateKey(uint8_t const * k, unsigned v)
    : value_(k, k+SIZE)
    , version_(v)
{
    valid_ = isValid();
}

std::string PrivateKey::toWif() const
{
    return Base58Check::encode(value_, version_);
}

std::string PrivateKey::toHex() const
{
    return Utility::vtox(value_);
}

bool PrivateKey::isValid()
{
    if (value_.size() != SIZE)
        return false;

    BIGNUM * i = BN_new();
    BN_bin2bn(&value_[0], (int)value_.size(), i);

    if (BN_is_zero(i))
    {
        BN_free(i);
        return false;
    }

    BIGNUM * maxPrivateKey = BN_new();
    BN_bin2bn(&MAX_PRIVATE_KEY[0], (int)MAX_PRIVATE_KEY_SIZE, maxPrivateKey);

    if (BN_cmp(i, maxPrivateKey) > 0)
    {
        BN_free(maxPrivateKey);
        BN_free(i);
        return false;
    }

    BN_free(maxPrivateKey);
    BN_free(i);
    return true;
}
