#include "VersionMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const VersionMessage::TYPE[] = "version";

VersionMessage::VersionMessage()
    : Message(TYPE)
{
}

VersionMessage::VersionMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void VersionMessage::serialize(std::vector<uint8_t> & out) const
{
}
