#pragma once

#include "network/Message.h"
#include "network/BlockHeader.h"

namespace Network
{

class HeadersMessage : public Message
{
public:

    HeadersMessage(BlockHeaderList const & headers);
    HeadersMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    BlockHeaderList const & headers_; //!< Block headers

    //! Command string for this message type
    static char const COMMAND[];

};

} // namespace Network
