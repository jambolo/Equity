#pragma once

#include "equity/Block.h"
#include "network/Message.h"

namespace Network
{

//! @brief Block message
//!
//! The block message contains a block and is sent in response to a getdata message which requests information from a block hash.

class BlockMessage : public Message
{
public:

    BlockMessage(Equity::Block const & block);
    BlockMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    Equity::Block block_;   //!< A block

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
