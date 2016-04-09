#pragma once

#include "network/Message.h"

namespace Network
{

//! A trasaction message.
//!
//! This message describes a bitcoin transaction, in reply to a get-data message.
//!
//! @sa GetDataMessage
//! @todo more to do for this message
class TxMessage : public Message
{
public:
    // Constructor
    TxMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    TxMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
