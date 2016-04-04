#pragma once

#include "network/Message.h"

namespace Network
{

//! @brief Check Order message
//!
//! This message was used for IP Transactions. As IP transactions have been deprecated, it is not supported.

class CheckOrderMessage : public Message
{
public:

    CheckOrderMessage();
    CheckOrderMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
