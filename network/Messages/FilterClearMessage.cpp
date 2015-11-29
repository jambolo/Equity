#include "FilterClearMessage.h"

using namespace Network;

char const FilterClearMessage::COMMAND[] = "filterclear";

FilterClearMessage::FilterClearMessage()
    : Message(COMMAND)
{
}

FilterClearMessage::FilterClearMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void FilterClearMessage::serialize(std::vector<uint8_t> & out) const
{
    Message::serialize(std::vector<uint8_t>(), out);
}
