#include "PrivateKey.h"

#include "Base58Check.h"
#include "Utility.h"

#include <openssl\bn.h>

#include <memory>

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
    unsigned version;
    valid_ = Base58Check::decode(wif, value_, version) && isValid();
}

PrivateKey::PrivateKey(std::vector<uint8_t> const & k)
    : value_(k)
{
    valid_ = isValid();
}

PrivateKey::PrivateKey(uint8_t const * k)
    : value_(k, k+SIZE)
{
    valid_ = isValid();
}

std::string PrivateKey::toWif(unsigned version) const
{
    return Base58Check::encode(value_, version);
}

std::string PrivateKey::toHex() const
{
    return Utility::vtox(value_);
}

bool PrivateKey::isValid()
{
    if (value_.size() != SIZE)
        return false;

    std::shared_ptr<BIGNUM> i(BN_new(), BN_free);
    BN_bin2bn(&value_[0], (int)value_.size(), i.get());

    if (BN_is_zero(i.get()))
    {
        return false;
    }

    std::shared_ptr<BIGNUM> maxPrivateKey(BN_new(), BN_free);
    BN_bin2bn(&MAX_PRIVATE_KEY[0], (int)MAX_PRIVATE_KEY_SIZE, maxPrivateKey.get());

    if (BN_cmp(i.get(), maxPrivateKey.get()) > 0)
    {
        return false;
    }

    return true;
}
