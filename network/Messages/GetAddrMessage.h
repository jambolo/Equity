#pragma once

#include "network/Message.h"

namespace Network
{

//! A get-address message.
//!
//! Requests a node for information about known active peers to help with finding potential nodes in the network.

class GetAddrMessage : public Message
{
public:

    GetAddrMessage();
    GetAddrMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
