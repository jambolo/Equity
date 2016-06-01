#include "Txid.h"

#include "p2p/Serialize.h"
#include "utility/Utility.h"

#include <algorithm>

using namespace Equity;

Txid::Txid(json const & json)
{
    std::vector<uint8_t> hash = Utility::fromHex(json.get<std::string>());
    if (hash_.size() != Crypto::SHA256_HASH_SIZE)
        throw P2p::DeserializationError();
    std::copy(hash.begin(), hash.end(), hash_.data());
}

Txid::Txid(uint8_t const * & in, size_t & size)
{
    if (size < Crypto::SHA256_HASH_SIZE)
        throw P2p::DeserializationError();
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

json Txid::toJson() const
{
    return P2p::toJson(hash_);
}
