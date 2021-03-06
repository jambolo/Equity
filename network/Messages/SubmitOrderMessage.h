#pragma once

#include "network/Message.h"
#include <nlohmann/json_fwd.hpp>

namespace Network
{
//! A submit-order message.
//!
//! @deprecated     This message was used for IP Transactions. As IP transactions have been deprecated, it is no
//!                 longer used.

class SubmitOrderMessage : public Message
{
public:

    // Constructor
    SubmitOrderMessage();

    // Deserialization constructor
    //!
    //! @param[in,out]  in      pointer to the next byte to deserialize
    //! @param[in,out]  size    number of bytes remaining in the serialized stream
    SubmitOrderMessage(uint8_t const * & in, size_t & size);

    //! @name Overrides Serializable
    //!@{
    virtual void           serialize(std::vector<uint8_t> & out) const override;
    virtual nlohmann::json toJson() const override;

    //!@}

    //! Message type
    static char const TYPE[];
};
} // namespace Network
