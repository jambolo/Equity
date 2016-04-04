#include "FilterClearMessage.h"

#include "utility/Debug.h"
#include "utility/Serialize.h"

using namespace Network;

char const FilterClearMessage::TYPE[] = "filterclear";

FilterClearMessage::FilterClearMessage()
    : Message(TYPE)
{
}

FilterClearMessage::FilterClearMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void FilterClearMessage::serialize(std::vector<uint8_t> & out) const
{
    NOT_YET_IMPLEMENTED();
}
