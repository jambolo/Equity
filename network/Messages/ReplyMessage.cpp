#include "ReplyMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const ReplyMessage::COMMAND[] = "reply";

ReplyMessage::ReplyMessage()
    : Message(COMMAND)
{
}

ReplyMessage::ReplyMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void ReplyMessage::serialize(std::vector<uint8_t> & out) const
{
}
