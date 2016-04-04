#include "InvMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const InvMessage::TYPE[] = "inv";

InvMessage::InvMessage(InventoryList const & inventory)
    : Message(TYPE)
    , inventory_(inventory)
{
}

InvMessage::InvMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , inventory_(Utility::VarArray<InventoryId>(in, size).value())
{
}

void InvMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::VarArray<InventoryId>(inventory_).serialize(out);
}
