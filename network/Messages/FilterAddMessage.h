#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief  Filter Add message
//!
//! This message is related to Bloom filtering of connections and is defined in BIP 0037.
//!
//! This message adds the given data element to the connections current filter without requiring a completely new one to be set.
//!
//! The given data element will be added to the Bloom filter. A filter must have been previously provided using filterload.
//! This command is useful if a new key or script is added to a clients wallet whilst it has connections to the network open,
//! it avoids the need to re-calculate and send an entirely new filter to every peer (though doing so is usually advisable to
//! maintain anonymity).

class FilterAddMessage : public Message
{
public:

    FilterAddMessage(std::vector<uint8_t> const & data);
    FilterAddMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! Data element to add to the current filter.
    std::vector<uint8_t> data_;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
