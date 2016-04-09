#include "SubmitOrderMessage.h"

#include "utility/Debug.h"

using namespace Network;

char const SubmitOrderMessage::TYPE[] = "submitorder";

SubmitOrderMessage::SubmitOrderMessage()
    : Message(TYPE)
{
    THIS_SHOULD_NEVER_HAPPEN();
}

SubmitOrderMessage::SubmitOrderMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    in += size;
    size = 0;
}

void SubmitOrderMessage::serialize(std::vector<uint8_t> & out) const
{
    THIS_SHOULD_NEVER_HAPPEN();
}
