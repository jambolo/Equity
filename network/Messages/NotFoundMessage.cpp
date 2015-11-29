#include "NotFoundMessage.h"

using namespace Network;

char const NotFoundMessage::COMMAND[] = "notfound";

NotFoundMessage::NotFoundMessage()
    : Message(COMMAND)
{
}

NotFoundMessage::NotFoundMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void Message::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
