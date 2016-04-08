#pragma once

#include "network/Message.h"
#include "equity/Block.h"

//! A headers message.
//!
//! Contains block headers in response to a get-headers message.
//! 
//! @note   The format of the data is a list of blocks that have had the transactions removed.
//! @sa     GetHeadersMessage

namespace Network
{

class HeadersMessage : public Message
{
public:

    HeadersMessage(Equity::BlockList const & blocks);
    HeadersMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    Equity::BlockList blocks_;   //!< Blocks with the transactions removed

    //! Message type
    static char const TYPE[];
};

} // namespace Network
