#include "NotFoundMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const NotFoundMessage::COMMAND[] = "notfound";

NotFoundMessage::NotFoundMessage(InventoryList const & missing)
    : Message(COMMAND)
    , missing_(missing)
{
}

NotFoundMessage::NotFoundMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
    , missing_(Utility::VarArray<InventoryId>(in, size).value())
{
}

void NotFoundMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Utility::VarArray<InventoryId>(missing_).serialize(payload);
    Message::serialize(payload, out);
}
