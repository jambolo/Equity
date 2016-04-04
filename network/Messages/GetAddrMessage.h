#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief  Get Address message
//!
//! Requests a node for information about known active peers to help with finding potential nodes in the network.

class GetAddrMessage : public Message
{
public:

    GetAddrMessage();
    GetAddrMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
