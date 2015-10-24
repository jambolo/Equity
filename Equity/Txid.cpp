#include "Txid.h"

#include "utility/Serialize.h"
#include "utility/Utility.h"


using namespace Equity;

Txid::Txid(std::string const & json)
    : Crypto::Sha256Hash(Utility::jtov(json))
{
}

Txid::Txid(uint8_t const *& in, size_t & size)
{
    if (size < Crypto::SHA256_HASH_SIZE)
    {
        in = nullptr;
        return;
    }
    insert(end(), in, in + Crypto::SHA256_HASH_SIZE);
    in += Crypto::SHA256_HASH_SIZE;
    size -= Crypto::SHA256_HASH_SIZE;
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


