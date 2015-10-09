#include "BitcoinAddress.h"

#include "Base58Check.h"
#include "crypto/Ripemd.h"
#include "crypto/Sha256.h"

using namespace Equity;
using namespace Crypto;


BitcoinAddress::BitcoinAddress(std::string const & s)
: string_(s)
{
    valid_ = Base58Check::decode(s, data_, network_);
}

BitcoinAddress::BitcoinAddress(Crypto::Ripemd160Hash const & k, unsigned n)
: data_(k)
, network_(n)
, string_(Base58Check::encode(k, n))
, valid_(true)
{
}
