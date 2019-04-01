#pragma once

#include "crypto/Sha256.h"
#include "equity/Block.h"
#include "network/Message.h"
#include "p2p/Serialize.h"
#include <nlohmann/json_fwd.hpp>

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
    // Constructor
    //!
    //! @param      header      block header
    //! @param      count       number of transactions in the block (including unmatched ones)
    //! @param      hashes      hashes in depth-first order
    //! @param      flags       flag bits

    MerkleBlockMessage(Equity::Block::Header const &  header,
                       uint32_t                       count,
                       Crypto::Sha256HashList const & hashes,
                       P2p::BitArray const &          flags);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    MerkleBlockMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    Equity::Block::Header header_;      //!< Block header
    uint32_t count_;                    //!< Number of transactions in the block (including unmatched ones)
    Crypto::Sha256HashList hashes_;     //!< Hashes in depth-first order
    P2p::BitArray flags_;               //!< Flag bits

    //! Message type
    static char const TYPE[];
};
} // namespace Network
