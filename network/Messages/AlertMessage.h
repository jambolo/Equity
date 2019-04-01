#pragma once

#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>
#include <vector>

namespace Network
{
//! An alert message.
//!
//! An alert is sent between nodes to send a general notification message throughout the network. If the alert can be confirmed
//! with the signature as having come from the core development group of the Bitcoin software, the message is suggested to be
//! displayed for end-users. Attempts to perform transactions, particularly automated transactions through the client, are
//! suggested to be halted. The text in the Message string should be relayed to log files and any user interfaces.

class AlertMessage : public Message
{
public:

    // Constructor
    //!
    //! @param  message     message to send
    AlertMessage(std::vector<uint8_t> const & message);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    AlertMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! The raw alert message data
    std::vector<uint8_t> message_;

    //! Message type
    static char const TYPE[];
};
} // namespace Network
