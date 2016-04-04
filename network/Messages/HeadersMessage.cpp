#include "HeadersMessage.h"

#include "equity/Block.h"
#include "utility/Serialize.h"

using namespace Network;

char const HeadersMessage::COMMAND[] = "headers";

HeadersMessage::HeadersMessage(Equity::BlockList const & blocks)
    : Message(COMMAND)
{
    // The message sends entire blocks with the transactions omitted.
    blocks_.reserve(blocks.size());
    for (auto const & block : blocks)
    {
        blocks_.emplace_back(block.header(), Equity::TransactionList());
    }
}

HeadersMessage::HeadersMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    blocks_ = Utility::VarArray<Equity::Block>(in, size).value();
}

void HeadersMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::VarArray<Equity::Block>(blocks_).serialize(out);
}
