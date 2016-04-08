#pragma once

#include "network/Message.h"

namespace Network
{

//! A pong message.
//!
//! Sent in response to a ping message
//!
//! @sa PingMessage

class PongMessage : public Message
{
public:

    PongMessage(uint64_t nonce);
    PongMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint64_t nonce_;    //!< Nonce to return

    //! Message type
    static char const TYPE[];
};

} // namespace Network
