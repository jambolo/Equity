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

    FilterClearMessage();
    FilterClearMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
