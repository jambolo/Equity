#pragma once

#include "network/Message.h"

namespace Network
{

//! A mempool message.
//!
//! Request a node to provide information about transactions it has verified but which have not yet confirmed. The response to
//! receiving this message is an inv message containing the transaction hashes for all the transactions in the node's mempool.
//!
//! This message is specified in BIP 35. With BIP 37, if a bloom filter is loaded, only transactions matching the filter are
//! returned.
//!
//! @sa     InventoryMessage

class MempoolMessage : public Message
{
public:
    // Constructor
    MempoolMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    MempoolMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
