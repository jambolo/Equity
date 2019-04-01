#include "CheckOrderMessage.h"

#include "utility/Debug.h"

using json = nlohmann::json;
using namespace Network;

char const CheckOrderMessage::TYPE[] = "checkorder";

CheckOrderMessage::CheckOrderMessage()
    : Message(TYPE)
{
    THIS_SHOULD_NEVER_HAPPEN();  // not supported
}

CheckOrderMessage::CheckOrderMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    in  += size;
    size = 0;
}

void CheckOrderMessage::serialize(std::vector<uint8_t> & out) const
{
    THIS_SHOULD_NEVER_HAPPEN();  // not supported
}

json Network::CheckOrderMessage::toJson() const
{
    THIS_SHOULD_NEVER_HAPPEN();  // not supported
    return json();
}
