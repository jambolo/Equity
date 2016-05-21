#include "Txid.h"

#include "p2p/Serialize.h"
#include "utility/Utility.h"

#include <algorithm>

using namespace Equity;

Txid::Txid(std::string const & json)
{
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

P2p::Serializable::cJSON_ptr Txid::toJson() const
{
    return P2p::toJson(hash_);
}
