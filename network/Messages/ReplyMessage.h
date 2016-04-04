#pragma once

#include "network/Message.h"

namespace Network
{

class ReplyMessage : public Message
{
public:

    ReplyMessage();
    ReplyMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Message type
    static char const TYPE[];
};

} // namespace Network
