#include "HeadersMessage.h"

#include "equity/Block.h"
#include "p2p/Serialize.h"

using namespace Network;

char const HeadersMessage::TYPE[] = "headers";

HeadersMessage::HeadersMessage(Equity::BlockList const & blocks)
    : Message(TYPE)
{
    // The message sends entire blocks, but with the transactions removed.
    blocks_.reserve(blocks.size());
    for (auto const & block : blocks)
    {
        blocks_.emplace_back(block.header(), Equity::TransactionList());
    }
}

HeadersMessage::HeadersMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    blocks_ = P2p::VarArray<Equity::Block>(in, size).value();
}

void HeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::VarArray<Equity::Block>(blocks_).serialize(out);
}
