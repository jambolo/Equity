#include "BlockMessage.h"

#include "equity/Block.h"
#include "p2p/Serialize.h"

using namespace Network;

char const BlockMessage::TYPE[] = "block";

BlockMessage::BlockMessage(Equity::Block const & block)
    : Message(TYPE)
    , block_(block)
{
}

BlockMessage::BlockMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , block_(in, size)
{
}

void BlockMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(block_, out);
}
