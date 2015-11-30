#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief  Ping message
//!
//! Sent to confirm that the connection to a node is still valid.
//!
//! @sa PongMessage

class PingMessage : public Message
{
public:

    PingMessage(uint64_t nonce);
    PingMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    uint64_t nonce_;    //!< Nonce

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
