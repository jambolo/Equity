#pragma once

#include "network/Inventory.h"
#include "network/Message.h"

namespace Network
{

//! @brief Get Data message
//!
//! The Get Data message is sent in response to an Inventory message in order to retrieve the content of a specific object, and is
//! usually sent after receiving an inventory message, after filtering known elements.

class GetDataMessage : public Message
{
public:

    GetDataMessage(InventoryList const & inventory);
    GetDataMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! List of items requested
    InventoryList inventory_;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
