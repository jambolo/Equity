#pragma once

#include "network/Message.h"

namespace Network
{

//! A reply message.
//!
//! @deprecated     This message was used for IP Transactions. As IP transactions have been deprecated, it is no
//!                 longer used.

class ReplyMessage : public Message
{
public:

    // Constructor
    ReplyMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    ReplyMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
