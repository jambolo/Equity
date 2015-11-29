#pragma once

#include "network/Message.h"
#include <network/Address.h>

namespace Network
{

class Address;

//! @brief Address message
//!
//! Provide information on known nodes of the network. Non-advertised nodes should be forgotten after typically 3 hours.

class AddrMessage : public Message
{
public:

    AddrMessage(std::vector<Address> const & addresses);
    AddrMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! Node addresses
    std::vector<Address> addresses_;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
