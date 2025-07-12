#include "PrivateKey.h"

#include "Base58Check.h"
#include "crypto/Ecc.h"
#include "crypto/Sha256.h"
#include "utility/Utility.h"

#include <memory>

using namespace Crypto;
using namespace Equity;

// A private key in mini-key format has exactly 30 characters including the S prefix
static size_t const MINI_KEY_FORMAT_SIZE = 30;

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
        Sha256Hash  check    = sha256(reinterpret_cast<uint8_t const *>(appended.data()), appended.length());
        if (check[0] != 0)
        {
            valid_ = false;
            return;
        }

        value_ = sha256(reinterpret_cast<uint8_t const *>(s.data()), s.length());
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

PrivateKey::PrivateKey(std::vector<uint8_t> const & k)
    : PrivateKey(k.data(), k.size())
{
}

PrivateKey::PrivateKey(uint8_t const * data, size_t size)
    : valid_(true)
    , compressed_(false)
{
    if (size != SIZE)
    {
        valid_ = false;
        return;
    }

    std::copy(data, data + size, value_.begin());
    valid_ = isValid();
}

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

std::string PrivateKey::toHex() const
{
    if (!valid_)
        return std::string();
    return Utility::toHex(value_);
}

bool PrivateKey::isValid()
{
    return valid_ /* && Ecc::privateKeyIsValid(value_)*/;
}
