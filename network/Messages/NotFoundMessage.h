#pragma once

#include "network/Inventory.h"
#include "network/Message.h"

namespace Network
{

//! A not-found message.
//!
//! Sent in response to a get-data message if any requested data items could not be relayed, for example, because the requested
//! transaction was not in the memory pool or relay set.
//!
//! @sa     GetDataMessage

class NotFoundMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  missing     missing items
    NotFoundMessage(InventoryList const & missing);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    NotFoundMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual cJSON_ptr toJson() const override;

    //!@}

    InventoryList missing_;     //!< Missing inventory

    //! Message type
    static char const TYPE[];
};

} // namespace Network
