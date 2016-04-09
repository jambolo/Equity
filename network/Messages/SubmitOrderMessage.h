#pragma once

#include "network/Message.h"

namespace Network
{

//! A submit-order message.
//!
//! @deprecated     This message was used for IP Transactions. As IP transactions have been deprecated, it is no
//!                 longer used.

class SubmitOrderMessage : public Message
{
public:

    // Constructor
    SubmitOrderMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    SubmitOrderMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
