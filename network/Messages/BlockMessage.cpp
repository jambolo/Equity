#include "BlockMessage.h"

#include "equity/Block.h"
#include "utility/Serialize.h"

using namespace Network;

char const BlockMessage::COMMAND[] = "block";

BlockMessage::BlockMessage(Equity::Block const & block)
    : Message(COMMAND)
    , block_(block)
{
}

BlockMessage::BlockMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
    , block_(in, size)
{
}

void BlockMessage::serialize(std::vector<uint8_t> & out) const
{
    block_.serialize(out);
}
