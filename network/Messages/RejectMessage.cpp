#include "RejectMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const RejectMessage::TYPE[] = "reject";

RejectMessage::RejectMessage()
    : Message(TYPE)
{
}

RejectMessage::RejectMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void RejectMessage::serialize(std::vector<uint8_t> & out) const
{
}
