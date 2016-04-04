#pragma once

#include "network/Inventory.h"
#include "network/Message.h"

namespace Network
{

//! @brief Not Found message
//!
//! Sent in response to a GetDataMessage if any requested data items could not be relayed, for example, because the requested
//! transaction was not in the memory pool or relay set.
//!
//! @sa     GetDataMessage

class NotFoundMessage : public Message
{
public:

    NotFoundMessage(InventoryList const & missing);
    NotFoundMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    InventoryList missing_;     //! Missing inventory

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
