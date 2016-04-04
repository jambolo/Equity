#include "SubmitOrderMessage.h"

#include <cassert>

using namespace Network;

char const SubmitOrderMessage::TYPE[] = "submitorder";

SubmitOrderMessage::SubmitOrderMessage()
    : Message(TYPE)
{
    assert(false);
}

SubmitOrderMessage::SubmitOrderMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    assert(false);
}

void SubmitOrderMessage::serialize(std::vector<uint8_t> & out) const
{
    assert(false);
}
