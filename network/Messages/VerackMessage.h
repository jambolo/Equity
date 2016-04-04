#pragma once

#include "network/Message.h"

namespace Network
{

class VerackMessage : public Message
{
public:

    VerackMessage();
    VerackMessage(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const override;

    //! Command string for this message type
    static char const COMMAND[];
};

} // namespace Network
