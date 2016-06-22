#include "PublicKey.h"

#include "PrivateKey.h"
#include "crypto/Ecc.h"
#include "utility/Utility.h"

using namespace Crypto;
using namespace Equity;

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : PublicKey(k.data(), k.size())
{
}

PublicKey::PublicKey(uint8_t const * data, size_t size)
    : valid_(true)
{
    if (!Ecc::publicKeyIsValid(data, size))
    {
        valid_ = false;
    }
    else
    {
        value_.assign(data, data + size);
        compressed_ = (data[0] != 4);
    }
}

PublicKey::PublicKey(PrivateKey const & k)
    : valid_(false)
{
    if (k.valid())
    {
        valid_ = Ecc::derivePublicKey(k.value(), value_, !k.compressed());
        compressed_ = k.compressed();
    }
}

std::string PublicKey::toHex() const
{
    return Utility::toHex(value_);
}
