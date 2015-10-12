#include "Address.h"

#include "Base58Check.h"
#include "PublicKey.h"
#include "PrivateKey.h"

#include "crypto/Ripemd.h"
#include "crypto/Sha256.h"

using namespace Equity;
using namespace Crypto;


Address::Address(std::string const & s)
{
    unsigned network;
    valid_ = Base58Check::decode(s, value_, network) && value_.size() == SIZE;
}

Address::Address(Crypto::Ripemd160Hash const & k)
    : value_(k)
    , valid_(k.size() == SIZE)
{
}

Address::Address(uint8_t const * k)
    : value_(k, k+SIZE)
    , valid_(true)
{
}

Address::Address(PublicKey const & publicKey)
    : value_(ripemd160(sha256(publicKey.value())))
    , valid_(true)
{
}

Address::Address(PrivateKey const & privateKey)
    : value_(ripemd160(sha256(PublicKey(privateKey).value())))
    , valid_(true)
{

}

std::string Address::toString(unsigned network) const
{
    return Base58Check::encode(value_, network);
}

