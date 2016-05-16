#pragma once

#include "equity/Block.h"
#include "network/Message.h"

namespace Network
{

//! A headers message.
//!
//! Contains block headers in response to a get-headers message.
//!
//! @note   The header used in this message is effectively a Block::Header plus an empty list of transactions. The
//!         block headers in this packet include a transaction count (a VarInt, so there can be more than 81 bytes
//!         per header) as opposed to the block headers that are hashed by miners. I don't know why a standard
//!         block header isn't used in this message.
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

    //! @name Overrides Serializable
    //!@{
    virtual void serialize(std::vector<uint8_t> & out) const override;
    virtual cJSON_ptr toJson() const override;

    //!@}

    Equity::BlockList blocks_;   //!< Blocks with the transactions removed

    //! Message type
    static char const TYPE[];
};

} // namespace Network
