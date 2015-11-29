#include "VerackMessage.h"

using namespace Network;

char const VerackMessage::COMMAND[] = "verack";

VerackMessage::VerackMessage()
    : Message(COMMAND)
{
}

VerackMessage::VerackMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void VerackMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
