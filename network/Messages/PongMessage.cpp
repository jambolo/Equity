#include "PongMessage.h"

using namespace Network;

char const PongMessage::COMMAND[] = "pong";

PongMessage::PongMessage()
    : Message(COMMAND)
{
}

PongMessage::PongMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void PongMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
