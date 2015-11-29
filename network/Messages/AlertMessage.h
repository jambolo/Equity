#pragma once

#include "network/Message.h"
#include <vector>

namespace Network
{

//! @brief Alert message
//!
//! An alert is sent between nodes to send a general notification message throughout the network. If the alert can be confirmed
//! with the signature as having come from the core development group of the Bitcoin software, the message is suggested to be
//! displayed for end-users. Attempts to perform transactions, particularly automated transactions through the client, are
//! suggested to be halted. The text in the Message string should be relayed to log files and any user interfaces.

class AlertMessage : public Message
{
public:

    AlertMessage(std::vector<uint8_t> const & message);
    AlertMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! The raw alert message data
    std::vector<uint8_t> message_;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
