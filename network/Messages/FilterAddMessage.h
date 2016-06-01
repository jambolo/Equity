#pragma once

#include "network/Message.h"

namespace Network
{

//! An add-filter message.
//!
//! This message adds the given data element to the connections current filter without requiring a completely new one to be set.
//!
//! The given data element will be added to the bloom filter. A filter must have been previously provided using a load-filter
//! message. This message is useful if a new key or script is added to a clients wallet whilst it has connections to the network
//! open, it avoids the need to re-calculate and send an entirely new filter to every peer (though doing so is usually advisable
//! to maintain anonymity).
//!
//! @note   This message is related to bloom filtering of connections and is defined in BIP 0037.
//! @sa     FilterClearMessage, FilterLoadMessage, MerkleBlockMessage

class FilterAddMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  data    data element
    FilterAddMessage(std::vector<uint8_t> const & data);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    FilterAddMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual json toJson() const override;

    //!@}

    //! Data element to add to the current filter.
    std::vector<uint8_t> data_;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
