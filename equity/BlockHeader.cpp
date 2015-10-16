#include "BlockHeader.h"

#include "utility/Serialize.h"


namespace Equity
{
    
    
    BlockHeader::BlockHeader(uint8_t const *& out, size_t & size)
    {
        version_ = Utility::deserialize<uint32_t>(out, size);
        if (out == NULL)
            return;
        previousBlock_ = Utility::deserializeBuffer(Crypto::SHA256_HASH_SIZE, out, size);
        if (out == NULL)
            return;
        merkleRoot_ = Utility::deserializeBuffer(Crypto::SHA256_HASH_SIZE, out, size);
        if (out == NULL)
            return;
        timestamp_ = Utility::deserialize<uint32_t>(out, size);
        if (out == NULL)
            return;
        target_ = Utility::deserialize<uint32_t>(out, size);
        if (out == NULL)
            return;
        nonce_ = Utility::deserialize<uint32_t>(out, size);
        if (out == NULL)
            return;
        count_ = Utility::deserialize<uint32_t>(out, size);
    }
        
    void BlockHeader::serialize(std::vector<uint8_t> & out)
    {
        Utility::serialize((uint32_t)version_, out);
        Utility::serialize(previousBlock_, out);
        Utility::serialize(merkleRoot_, out);
        Utility::serialize(timestamp_, out);
        Utility::serialize(target_, out);
        Utility::serialize(nonce_, out);
        Utility::serialize(count_, out);
    }

} // namespace Equity
