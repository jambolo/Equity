#pragma once

#include "equity/Block.h"
#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

namespace Network
{
//! A block message.
//!
//! The block message contains a block and is sent in response to a getdata message which requests information from a block hash.

class BlockMessage : public Message
{
public:

    // Constructor
    //!
    //! @param  block   a block
    BlockMessage(Equity::Block const & block);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    BlockMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    Equity::Block block_;   //!< A block

    //! Message type
    static char const TYPE[];
};
} // namespace Network
