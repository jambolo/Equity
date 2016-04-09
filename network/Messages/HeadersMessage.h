#pragma once

#include "network/Message.h"
#include "equity/Block.h"

namespace Network
{

//! A headers message.
//!
//! Contains block headers in response to a get-headers message.
//! 
//! @note   The format of the data is a list of blocks that have had the transactions removed.
//! @sa     GetHeadersMessage

class HeadersMessage : public Message
{
public:
    // Constructor
    //!
    //! @param  blocks      list of blocks
    HeadersMessage(Equity::BlockList const & blocks);

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    HeadersMessage(uint8_t const * & in, size_t & size);

    //! Overrides Serializable
    virtual void serialize(std::vector<uint8_t> & out) const override;

    Equity::BlockList blocks_;   //!< Blocks with the transactions removed

    //! Message type
    static char const TYPE[];
};

} // namespace Network
