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
    // Constructor
    //!
    //! @param  inventory   requested items
    GetDataMessage(InventoryList const & inventory);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    GetDataMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    InventoryList inventory_;    //!< List of items requested

    //! Message type
    static char const TYPE[];
};

} // namespace Network
