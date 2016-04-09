#pragma once

#include "network/Message.h"

namespace Network
{

//! A ping message.
//!
//! Sent to confirm that the connection to a node is still valid.
//!
//! @sa PongMessage

class PingMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  nonce       a nonce
    PingMessage(uint64_t nonce);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    PingMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint64_t nonce_;    //!< Nonce

    //! Message type
    static char const TYPE[];
};

} // namespace Network
