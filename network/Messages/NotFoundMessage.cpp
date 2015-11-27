#include "network/Message.h"

using namespace Network;

Message::Message(uint32_t m)
    : Message(m, "")
{
}

Message::Message(uint8_t const * & in, size_t & size)
    : Message(in, size)
{
}

void Message::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
