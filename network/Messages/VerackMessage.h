#pragma once

#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

namespace Network
{
//! a version-acknowledge message.
//!
//! The version-acknowledge message is sent in reply to a version message. This message contains no payload.

class VerackMessage : public Message
{
public:
    // Constructor
    VerackMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    VerackMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! Message type
    static char const TYPE[];
};
} // namespace Network
