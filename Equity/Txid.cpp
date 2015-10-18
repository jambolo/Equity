#include "Txid.h"

#include "utility/Serialize.h"
#include "utility/Utility.h"

using namespace Equity;

Txid::Txid(std::string const & json)
    : Crypto::Sha256Hash(Utility::jtov(json))
{
}

void Txid::serialize(std::vector<uint8_t> & out) const
{
    out.insert(out.end(), begin(), end());
}

std::string Txid::toHex() const
{
    return Utility::vtox(*this);
}


std::string Txid::toJson() const
{
    return Utility::toJson(*this);
}


