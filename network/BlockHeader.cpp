#include "BlockHeader.h"

#include "crypto/Sha256.h"
#include "utility/Serialize.h"

namespace Network
{

BlockHeader::BlockHeader(uint8_t const * & in, size_t & size)
{
    version_ = Utility::deserialize<uint32_t>(in, size);
    previousBlock_ = Utility::deserializeArray<uint8_t>(Crypto::SHA256_HASH_SIZE, in, size);
    merkleRoot_ = Utility::deserializeArray<uint8_t>(Crypto::SHA256_HASH_SIZE, in, size);
    timestamp_ = Utility::deserialize<uint32_t>(in, size);
    target_ = Utility::deserialize<uint32_t>(in, size);
    nonce_ = Utility::deserialize<uint32_t>(in, size);
    count_ = Utility::deserialize<uint32_t>(in, size);
}

void BlockHeader::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize((uint32_t)version_, out);
    Utility::serializeArray(previousBlock_, out);
    Utility::serializeArray(merkleRoot_, out);
    Utility::serialize(timestamp_, out);
    Utility::serialize(target_, out);
    Utility::serialize(nonce_, out);
    Utility::serialize(count_, out);
}

} // namespace Network
