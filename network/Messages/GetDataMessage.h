#pragma once

#include "network/Inventory.h"
#include "network/Message.h"

namespace Network
{

//! A get-data message.
//!
//! The get-data message is sent in response to an inventory message in order to retrieve the content of a specific object, and is
//! usually sent after receiving an inventory message, after filtering known elements.
//!
//! @sa     InventoryMessage

class GetDataMessage : public Message
{
public:

    GetDataMessage(InventoryList const & inventory);
    GetDataMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! List of items requested
    InventoryList inventory_;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
