#pragma once

#include "crypto/Sha256.h"

#include <cstdint>
#include <vector>

namespace Equity
{
    

class BlockHeader
{
public:
    BlockHeader(uint8_t const *& out, size_t & size);
    
    void serialize(std::vector<uint8_t> & out);
    
    int32_t version() const { return version_; }
    Crypto::Sha256Hash previousBlock() const { return previousBlock_; }
    Crypto::Sha256Hash merkleRoot() const { return merkleRoot_; }
    uint32_t timestamp() const { return timestamp_; }
    uint32_t target() const { return target_; }
    uint32_t nonce() const { return nonce_; }
    uint64_t count() const { return count_; }

private:
    
    int32_t version_;
    Crypto::Sha256Hash previousBlock_;
    Crypto::Sha256Hash merkleRoot_;
    uint32_t timestamp_;
    uint32_t target_;
    uint32_t nonce_;
    uint64_t count_;
};

    
} // namespace Equity
