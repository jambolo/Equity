#pragma once

#include "network/Message.h"

namespace Network
{

//! A clear-filter message.
//!
//! This message deletes the current filter and goes back to regular pre-BIP37 usage
//!
//! @note   This message is related to bloom filtering of connections and is defined in BIP 0037.
//! @sa     FilterAddMessage, FilterLoadMessage, MerkleBlockMessage

class FilterClearMessage : public Message
{
public:
    // Constructor
    FilterClearMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    FilterClearMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
