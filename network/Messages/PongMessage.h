#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief  Pong message
//!
//! Sent in response to a PingMessage
//!
//! @sa PingMessage

class PongMessage : public Message
{
public:

    PongMessage(uint64_t nonce);
    PongMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    uint64_t nonce_;    //!< Nonce to return

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
