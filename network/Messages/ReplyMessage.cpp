#include "ReplyMessage.h"

#include "p2p/Serialize.h"
#include "utility/Debug.h"

using namespace Network;

char const ReplyMessage::TYPE[] = "reply";

ReplyMessage::ReplyMessage()
    : Message(TYPE)
{
    THIS_SHOULD_NEVER_HAPPEN();
}

ReplyMessage::ReplyMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    in += size;
    size = 0;
}

void ReplyMessage::serialize(std::vector<uint8_t> & out) const
{
    THIS_SHOULD_NEVER_HAPPEN();
}
