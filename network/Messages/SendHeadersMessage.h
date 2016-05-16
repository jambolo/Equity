#pragma once
#include "network/Message.h"

namespace Network
{

//! A send-headers message.
//!
//! Upon receipt of this message, the node is be permitted, but not required, to announce new blocks by headers
//! messages (instead of inventory messages).
//!
//! This message is supported by the protocol version >= 70012 or Bitcoin Core version >= 0.12.0.
//!
//! @sa https://github.com/bitcoin/bips/blob/master/bip-0130.mediawiki

class SendHeadersMessage : public Message
{
public:
    // Constructor
    SendHeadersMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    SendHeadersMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual cJSON_ptr toJson() const override;

    //!@}

    //! Message type
    static char const TYPE[];
};

} // namespace Network
