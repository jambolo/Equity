#pragma once

#include "equity/BlockHeader.h"
#include "network/Message.h"

namespace Network
{

//! @brief block Message
//!
//! The block message is sent in response to a getdata message which requests transaction information from a block hash.

class BlockMessage : public Message
{
public:

    BlockMessage(Equity::BlockHeader const & header);
    BlockMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    Equity::BlockHeader header_;
    static char const COMMAND[];
};

} // namespace Network
