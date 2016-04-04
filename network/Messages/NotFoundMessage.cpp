#include "NotFoundMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const NotFoundMessage::TYPE[] = "notfound";

NotFoundMessage::NotFoundMessage(InventoryList const & missing)
    : Message(TYPE)
    , missing_(missing)
{
}

NotFoundMessage::NotFoundMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , missing_(P2p::VarArray<InventoryId>(in, size).value())
{
}

void NotFoundMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::VarArray<InventoryId>(missing_).serialize(out);
}
