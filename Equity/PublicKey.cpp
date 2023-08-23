#include "PublicKey.h"

#include "crypto/Ecc.h"
#include "PrivateKey.h"
#include "utility/Utility.h"

using namespace Crypto;
using namespace Equity;

PublicKey::PublicKey(std::vector<uint8_t> const & k)
    : PublicKey(k.data(), k.size())
{
}

PublicKey::PublicKey(uint8_t const * data, size_t size)
    : valid_(false)
    , compressed_(data[0] != 4)

{
    if (Ecc::publicKeyIsValid(data, size))
    {
        valid_ = true;
        value_.assign(data, data + size);
    }
}

PublicKey::PublicKey(PrivateKey const & k)
    : valid_(false)
    , compressed_(k.compressed())
{
    if (k.valid())
    {
        valid_ = Ecc::derivePublicKey(k.value(), value_, !k.compressed());
    }
}

std::string PublicKey::toHex() const
{
    return Utility::toHex(value_);
}
