#pragma once

#include "network/Message.h"
#include <network/Address.h>

namespace Network
{

class Address;

//! An address message.
//!
//! Provides information on known nodes of the network. Non-advertised nodes should be forgotten after typically 3 hours.

class AddressMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  addresses   Vector of node decriptions
    AddressMessage(std::vector<Address> const & addresses);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    AddressMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    //! Node descriptions
    std::vector<Address> addresses_;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
