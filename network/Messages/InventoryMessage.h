#pragma once

#include "network/Inventory.h"
#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

namespace Network
{
//! An inventory message.
//!
//! Contains hashes corresponding to transactions and blocks. Can be sent unsolicited or in response to a get-blocks message.
//!
//! @sa     GetBlocksMessage

class InventoryMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  inventory       list of hashes
    InventoryMessage(InventoryList const & inventory);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    InventoryMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    InventoryList inventory_;   //!< List of hashes

    //! Message type
    static char const TYPE[];
};
} // namespace Network
