#include "PrivateKey.h"

#include "Base58Check.h"
#include "crypto/Sha256.h"
#include "utility/Utility.h"

#include <openssl/bn.h>

#include <memory>

using namespace Equity;

static uint8_t const MAX_PRIVATE_KEY[] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
    0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B,
    0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
};

// A private key in mini-key format has exactly 30 characters including the S prefix
static size_t const MINI_KEY_FORMAT_SIZE = 30;

//! The input can be in either the WIF format or the mini-key format. If the format is invalid or the value of the
//! key is invalid, the contructed object is marked as invalid.
//!
//! @param  s   string form of the private key

PrivateKey::PrivateKey(std::string const & s)
    : valid_(true)
    , compressed_(false)
{
    if (s.size() == MINI_KEY_FORMAT_SIZE)
    {
        if (s[0] != 'S')
        {
            valid_ = false;
            return;
        }
        std::string appended = s + '?';
        Crypto::Sha256Hash check = Crypto::sha256(reinterpret_cast<uint8_t const *>(appended.c_str()), appended.length());
        if (check[0] != 0)
        {
            valid_ = false;
            return;
        }

        value_ = Crypto::sha256(reinterpret_cast<uint8_t const *>(s.c_str()), s.length());
    }
    else
    {
        unsigned version;       // Throw away the version byte?
        std::vector<uint8_t> decoded;
        valid_ = Base58Check::decode(s, decoded, version);
        if (!valid_)
            return;

        if (decoded.size() == SIZE + 1 && decoded.back() == COMPRESSED_FLAG)
        {
            compressed_ = true;
            decoded.resize(SIZE);
        }
        else if (decoded.size() != SIZE)
        {
            valid_ = false;
            return;
        }

        std::copy(decoded.begin(), decoded.end(), value_.begin());
    }
    valid_ = isValid();
}

//! The object is constructed from the bytes in the given vector. If the size of the vector is not SIZE or the
//! value is not valid, the constructed object is marked as invalid.
//!
//! @param  k   vector of bytes

PrivateKey::PrivateKey(std::vector<uint8_t> const & k)
    : PrivateKey(k.data(), k.data() + k.size())
{
}

//! The object is constructed from the bytes in the range [begin, end). If the size of the range is not 
//! SIZE bytes or the value is not valid, the constructed object is marked as invalid.
//!
//! @param  begin   pointer to start of the range
//! @param  end     pointer to the end of the range

PrivateKey::PrivateKey(uint8_t const * begin, uint8_t const * end)
    : valid_(true)
    , compressed_(false)
{
    if (end != begin + SIZE)
    {
        valid_ = false;
        return;
    }

    std::copy(begin, end, value_.begin());
    valid_ = isValid();
}

//! @param  version     version byte to be prepended (typically 0x80)
//!
//! @return     a string containing the private key in WIF format, or empty if the object is invalid

std::string PrivateKey::toWif(unsigned version) const
{
    if (!valid_)
        return std::string();

    if (compressed_)
    {
        std::vector<uint8_t> extended(value_.begin(), value_.end());
        extended.push_back(COMPRESSED_FLAG);
        return Base58Check::encode(extended, version);
    }
    else
    {
        return Base58Check::encode(value_, version);
    }
}

//!
//! @return     a string containing the private key in hex format, or empty if the object is invalid
std::string PrivateKey::toHex() const
{
    if (!valid_)
        return std::string();
    return Utility::toHex(value_);
}

bool PrivateKey::isValid()
{
    if (!valid_)
        return false;

    std::shared_ptr<BIGNUM> i(BN_new(), BN_free);
    BN_bin2bn(value_.data(), (int)value_.size(), i.get());

    if (BN_is_zero(i.get()))
        return false;

    std::shared_ptr<BIGNUM> maxPrivateKey(BN_new(), BN_free);
    BN_bin2bn(MAX_PRIVATE_KEY, (int)sizeof(MAX_PRIVATE_KEY), maxPrivateKey.get());

    if (BN_cmp(i.get(), maxPrivateKey.get()) > 0)
        return false;

    return true;
}
