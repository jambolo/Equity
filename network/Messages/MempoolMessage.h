#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief  Mempool message
//!
//! Request a node to provide information about transactions it has verified but which have not yet confirmed. The response to
//! receiving this message is an inv message containing the transaction hashes for all the transactions in the node's mempool.
//!
//! This message is specified in BIP 35. With BIP 37, if a bloom filter is loaded, only transactions matching the filter are
//! returned.
//!
//! @sa     InvMessage

class MempoolMessage : public Message
{
public:

    MempoolMessage();
    MempoolMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
