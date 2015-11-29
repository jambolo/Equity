#include "HeadersMessage.h"

#include "network/BlockHeader.h"

using namespace Network;

char const HeadersMessage::COMMAND[] = "headers";

HeadersMessage::HeadersMessage(BlockHeaderList const & headers)
    : Message(COMMAND)
{
}

HeadersMessage::HeadersMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void HeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
