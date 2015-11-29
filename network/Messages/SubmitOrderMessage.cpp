#include "SubmitOrderMessage.h"

#include <cassert>

using namespace Network;

char const SubmitOrderMessage::COMMAND[] = "submitorder";

SubmitOrderMessage::SubmitOrderMessage()
    : Message(COMMAND)
{
    assert(false);
}

SubmitOrderMessage::SubmitOrderMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    assert(false);
}

void Message::serialize(std::vector<uint8_t> & out) const
{
    assert(false);
}
