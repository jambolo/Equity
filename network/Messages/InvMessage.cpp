#include "InvMessage.h"

using namespace Network;

char const InvMessage::COMMAND[] = "inv";

InvMessage::InvMessage()
    : Message(COMMAND)
{
}

InvMessage::InvMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void InvMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
