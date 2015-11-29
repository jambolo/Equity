#include "BlockMessage.h"

using namespace Network;

char const BlockMessage::COMMAND[] = "block";

BlockMessage::BlockMessage(BlockHeader const & header)
    : Message(COMMAND)
    , header_(header)
{
}

BlockMessage::BlockMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
    , header_(in, size)
{
}

void BlockMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    header_.serialize(payload);
    Message::serialize(payload, out);
}
