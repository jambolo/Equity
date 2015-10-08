#include "BitcoinAddress.h"

#include "Base58Check.h"
#include "crypto/Ripemd.h"
#include "crypto/Sha256.h"

using namespace Equity;
using namespace Crypto;


BitcoinAddress::BitcoinAddress(std::string const & s)
{


}

BitcoinAddress::BitcoinAddress(Crypto::Ripemd::Hash const & k, unsigned network)
    : data_(k)
    , string_(Base58Check()(data_, network))
{
}
