#include "GetAddrMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const GetAddrMessage::TYPE[] = "getaddr";

GetAddrMessage::GetAddrMessage()
    : Message(TYPE)
{
}

GetAddrMessage::GetAddrMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void GetAddrMessage::serialize(std::vector<uint8_t> & out) const
{
}
