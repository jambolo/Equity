#include "PingMessage.h"

using namespace Network;

char const PingMessage::COMMAND[] = "ping";

PingMessage::PingMessage()
    : Message(COMMAND)
{
}

PingMessage::PingMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void PingMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
