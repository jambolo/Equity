#pragma once

#include "crypto/Sha256.h"
#include "equity/Block.h"
#include "network/Message.h"
#include "network/Utility.h"

namespace Network
{

//! A filtered-block message.
//!
//! Contains information about a block and a filtered list of transactions in the block
//!
//! @note   This message is related to bloom filtering of connections and is defined in BIP 0037.
//! @sa     FilterAddMessage, FilterClearMessage, FilterLoadMessage

class MerkleBlockMessage : public Message
{
public:

    MerkleBlockMessage(Equity::Block::Header const &  header,
                       uint32_t                       count,
                       Crypto::Sha256HashList const & hashes,
                       BitArray const &               flags);
    MerkleBlockMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    Equity::Block::Header header_;
    uint32_t count_;
    Crypto::Sha256HashList hashes_;
    BitArray flags_;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
