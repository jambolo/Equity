#include "InvMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const InvMessage::TYPE[] = "inv";

InvMessage::InvMessage(InventoryList const & inventory)
    : Message(TYPE)
    , inventory_(inventory)
{
}

InvMessage::InvMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , inventory_(P2p::VarArray<InventoryId>(in, size).value())
{
}

void InvMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::VarArray<InventoryId>(inventory_).serialize(out);
}
