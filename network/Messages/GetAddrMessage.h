#pragma once

#include "network/Message.h"

namespace Network
{

//! A get-address message.
//!
//! Requests a node for information about known active peers to help with finding potential nodes in the network.

class GetAddrMessage : public Message
{
public:
    // Constructor
    GetAddrMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    GetAddrMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
