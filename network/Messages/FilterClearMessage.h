#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief Filter Clear message
//!
//! This message is related to Bloom filtering of connections and is defined in BIP 0037.
//!
//! This message deletes the current filter and goes back to regular pre-BIP37 usage

class FilterClearMessage : public Message
{
public:

    FilterClearMessage();
    FilterClearMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
