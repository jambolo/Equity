#include "InvMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const InvMessage::COMMAND[] = "inv";

InvMessage::InvMessage(InventoryList const & inventory)
    : Message(COMMAND)
    , inventory_(inventory)
{
}

InvMessage::InvMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
    , inventory_(Utility::VarArray<InventoryId>(in, size).value())
{
}

void InvMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Utility::VarArray<InventoryId>(inventory_).serialize(payload);
    Message::serialize(payload, out);
}
