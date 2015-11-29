#include "CheckOrderMessage.h"

#include <cassert>

using namespace Network;

char const CheckOrderMessage::COMMAND[] = "checkorder";

CheckOrderMessage::CheckOrderMessage()
    : Message(COMMAND)
{
    assert(false);  // not supported
}

CheckOrderMessage::CheckOrderMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    assert(false);  // not supported
}

void CheckOrderMessage::serialize(std::vector<uint8_t> & out) const
{
    assert(false);  // not supported
}
