#include "VersionMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const VersionMessage::COMMAND[] = "version";

VersionMessage::VersionMessage()
    : Message(COMMAND)
{
}

VersionMessage::VersionMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void VersionMessage::serialize(std::vector<uint8_t> & out) const
{
}
