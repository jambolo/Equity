#include "GetDataMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const GetDataMessage::TYPE[] = "getdata";

GetDataMessage::GetDataMessage(InventoryList const & inventory)
    : Message(TYPE)
    , inventory_(inventory)
{
}

GetDataMessage::GetDataMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    inventory_ = P2p::VarArray<InventoryId>(in, size).value();
}

void GetDataMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::VarArray<InventoryId>(inventory_).serialize(out);
}
