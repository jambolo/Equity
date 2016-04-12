#include "Address.h"

#include "Base58Check.h"
#include "PrivateKey.h"
#include "PublicKey.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha256.h"

using namespace Equity;
using namespace Crypto;

Address::Address(std::string const & s)
{
    unsigned network;
    valid_ = Base58Check::decode(s.c_str(), value_.data(), value_.size(), network);
}

Address::Address(Crypto::Ripemd160Hash const & k)
    : value_(k)
    , valid_(true)
{
}

Address::Address(uint8_t const * data, size_t size)
    : valid_(true)
{
    if (size != SIZE)
    {
        valid_ = false;
        return;
    }
    std::copy(data, data + size, value_.data());
}

Address::Address(PublicKey const & publicKey)
    : value_(ripemd160(sha256(publicKey.value())))
    , valid_(true)
{
}

std::string Address::toString(unsigned network) const
{
    return Base58Check::encode(value_, network);
}
