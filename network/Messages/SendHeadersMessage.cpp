#include "SendHeadersMessage.h"

using namespace Network;

char const SendHeadersMessage::TYPE[] = "sendheaders";

SendHeadersMessage::SendHeadersMessage()
    : Message(TYPE)
{
    // This message has no payload
}

SendHeadersMessage::SendHeadersMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    // This message has no payload
}

void SendHeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    // This message has no payload
}
