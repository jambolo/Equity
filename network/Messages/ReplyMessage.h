#pragma once

#include "network/Message.h"

namespace Network
{

class Message : public Message
{
public:

    Message(uint32_t m, std::string const & c);
    Message(uint8_t const * & in, size_t & size);

    virtual void serialize(std::vector<uint8_t> & out) const;

private:

};

} // namespace Network
