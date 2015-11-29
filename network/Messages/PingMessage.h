#pragma once

#include "network/Message.h"

namespace Network
{

class PingMessage : public Message
{
public:

    PingMessage();
    PingMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
