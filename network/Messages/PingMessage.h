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

    PingMessage(uint64_t nonce);
    PingMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint64_t nonce_;    //!< Nonce

    //! Message type
    static char const TYPE[];
};

} // namespace Network
