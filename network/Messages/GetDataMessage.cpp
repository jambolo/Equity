#include "GetDataMessage.h"

#include "utility/Serialize.h"

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
    inventory_ = Utility::VarArray<InventoryId>(in, size).value();
}

void GetDataMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::VarArray<InventoryId>(inventory_).serialize(out);
}
