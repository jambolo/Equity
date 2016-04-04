#pragma once

#include "network/Message.h"
#include "network/Inventory.h"

//! @brief  Inventory message
//!
//! Contains hashes corresponding to transactions and blocks. Can be sent unsolicited or in response to a GetBlocksMessage.
//!
//! @sa     GetBlocksMessage

namespace Network
{

class InvMessage : public Message
{
public:

    InvMessage(InventoryList const & inventory);
    InvMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    InventoryList inventory_;   //! Inventory

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
