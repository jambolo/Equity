#include "RejectMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const RejectMessage::COMMAND[] = "reject";

RejectMessage::RejectMessage()
    : Message(COMMAND)
{
}

RejectMessage::RejectMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void RejectMessage::serialize(std::vector<uint8_t> & out) const
{
}
