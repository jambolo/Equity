#include "GetAddrMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const GetAddrMessage::COMMAND[] = "getaddr";

GetAddrMessage::GetAddrMessage()
    : Message(COMMAND)
{
}

GetAddrMessage::GetAddrMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void GetAddrMessage::serialize(std::vector<uint8_t> & out) const
{
}
