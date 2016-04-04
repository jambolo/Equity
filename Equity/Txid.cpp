#include "Txid.h"

#include "p2p/Serialize.h"
#include "utility/Utility.h"

#include <algorithm>

using namespace Equity;

Txid::Txid(std::string const & json)
{
    std::vector<uint8_t> h = Utility::fromJson(json);
    if (h.size() == hash_.size())
        std::copy(h.begin(), h.end(), hash_.begin());
}

Txid::Txid(uint8_t const * & in, size_t & size)
{
    if (size < Crypto::SHA256_HASH_SIZE)
        throw DeserializationError();
    std::copy(in, in + hash_.size(), hash_.data());
    std::reverse(hash_.begin(), hash_.end());   // Txid's are stored and displayed as big-endian, but serialized as little-endian.
    in += Crypto::SHA256_HASH_SIZE;
    size -= Crypto::SHA256_HASH_SIZE;
}

void Txid::serialize(std::vector<uint8_t> & out) const
{
    out.insert(out.end(), hash_.rbegin(), hash_.rend());    // Txid's are stored and displayed as big-endian, but serialized as
                                                            // little-endian.
}

std::string Txid::toHex() const
{
    return Utility::toHex(hash_);
}

std::string Txid::toJson() const
{
    return Utility::toJson(hash_);
}
