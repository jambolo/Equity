#pragma once

#include "network/Message.h"
#include "network/Inventory.h"

//! An inventory message.
//!
//! Contains hashes corresponding to transactions and blocks. Can be sent unsolicited or in response to a get-blocks message.
//!
//! @sa     GetBlocksMessage

namespace Network
{

class InventoryMessage : public Message
{
public:

    InventoryMessage(InventoryList const & inventory);
    InventoryMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    InventoryList inventory_;   //! Inventory

    //! Message type
    static char const TYPE[];
};

} // namespace Network
