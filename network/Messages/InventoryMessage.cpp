#include "InventoryMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const InventoryMessage::TYPE[] = "inv";

InventoryMessage::InventoryMessage(InventoryList const & inventory)
    : Message(TYPE)
    , inventory_(inventory)
{
}

InventoryMessage::InventoryMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    inventory_ = P2p::VarArray<InventoryId>(in, size).value();
}

void InventoryMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(P2p::VarArray<InventoryId>(inventory_), out);
}
