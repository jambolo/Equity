#include "ReplyMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const ReplyMessage::TYPE[] = "reply";

ReplyMessage::ReplyMessage()
    : Message(TYPE)
{
}

ReplyMessage::ReplyMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void ReplyMessage::serialize(std::vector<uint8_t> & out) const
{
}
