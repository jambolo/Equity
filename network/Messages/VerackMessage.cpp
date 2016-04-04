#include "VerackMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const VerackMessage::TYPE[] = "verack";

VerackMessage::VerackMessage()
    : Message(TYPE)
{
}

VerackMessage::VerackMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void VerackMessage::serialize(std::vector<uint8_t> & out) const
{
}
