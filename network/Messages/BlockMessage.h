#pragma once

#include "network/BlockHeader.h"
#include "network/Message.h"

namespace Network
{

//! @brief Block message
//!
//! The block message is sent in response to a getdata message which requests transaction information from a block hash.

class BlockMessage : public Message
{
public:

    BlockMessage(BlockHeader const & header);
    BlockMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    BlockHeader header_;    //!< A block header

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
