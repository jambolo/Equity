#include "FilterClearMessage.h"

#include "p2p/Serialize.h"
#include "utility/Debug.h"

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
    // Nothing to serialize
}

json Network::FilterClearMessage::toJson() const
{
	return json();
}
