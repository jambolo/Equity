#include "NotFoundMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const NotFoundMessage::TYPE[] = "notfound";

NotFoundMessage::NotFoundMessage(InventoryList const & missing)
    : Message(TYPE)
    , missing_(missing)
{
}

NotFoundMessage::NotFoundMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , missing_(Utility::VarArray<InventoryId>(in, size).value())
{
}

void NotFoundMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::VarArray<InventoryId>(missing_).serialize(out);
}
