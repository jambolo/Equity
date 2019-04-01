#pragma once

#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

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

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    uint64_t nonce_;    //!< Nonce

    //! Message type
    static char const TYPE[];
};
} // namespace Network
