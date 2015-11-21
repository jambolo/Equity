#include "Txid.h"

#include "utility/Serialize.h"
#include "utility/Utility.h"

#include <algorithm>

using namespace Equity;

Txid::Txid(std::string const & json)
    : Crypto::Sha256Hash(Utility::fromJson(json))
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
    std::reverse(begin(), end());   // Txid's are stored and displayed as big-endian, but serialized as little-endian.
    in += Crypto::SHA256_HASH_SIZE;
    size -= Crypto::SHA256_HASH_SIZE;
}

void Txid::serialize(std::vector<uint8_t> & out) const
{
    out.insert(out.end(), rbegin(), rend());    // Txid's are stored and displayed as big-endian, but serialized as little-endian.
}

std::string Txid::toHex() const
{
    return Utility::toHex(*this);
}


std::string Txid::toJson() const
{
    return Utility::toJson(*this);
}


