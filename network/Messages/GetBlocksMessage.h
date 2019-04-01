#pragma once

#include "crypto/Sha256.h"
#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

namespace Network
{
//! A get-blocks message.
//!
//! Requests an inventory message containing the list of blocks starting right after the last known hash in the block locator
//! object, up to hash_stop or 500 blocks, whichever comes first.
//!
//! @sa InventoryMessage

class GetBlocksMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  version     protocol version
    //! @param  hashes      requested blocks
    //! @param  last        last hash to return
    GetBlocksMessage(uint32_t version, Crypto::Sha256HashList const & hashes, Crypto::Sha256Hash const & last);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    GetBlocksMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    uint32_t version_;              //!< The protocol version
    Crypto::Sha256HashList hashes_; //!< List of block hashes to look for
    Crypto::Sha256Hash last_;       //!< Last hash to return

    //! Message type
    static char const TYPE[];
};
} // namespace Network
