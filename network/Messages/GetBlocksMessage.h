#pragma once

#include "network/Message.h"
#include "crypto/Sha256.h"

namespace Network
{

//! @brief Get Blocks message
//!
//! Requests an inventory message containing the list of blocks starting right after the last known hash in the block locator
//! object, up to hash_stop or 500 blocks, whichever comes first.
//!
//! @sa InvMessage

class GetBlocksMessage : public Message
{
public:

    GetBlocksMessage(uint32_t version, Crypto::Sha256HashList const & hashes, Crypto::Sha256Hash const & last);
    GetBlocksMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint32_t version_;              //!< The protocol version
    Crypto::Sha256HashList hashes_; //!< List of block hashes to look for
    Crypto::Sha256Hash last_;       //!< Last hash to return

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
