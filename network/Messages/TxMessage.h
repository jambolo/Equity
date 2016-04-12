#pragma once

#include "network/Message.h"

//! @todo rename file to transactionMessage
namespace Network
{

//! A trasaction message.
//!
//! This message describes a bitcoin transaction, in reply to a get-data message.
//!
//! @sa GetDataMessage
//! @todo more to do for this message

class TransactionMessage : public Message
{
public:
    // Constructor
    TransactionMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    TransactionMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
