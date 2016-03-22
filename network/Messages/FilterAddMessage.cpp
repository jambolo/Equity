#include "FilterAddMessage.h"

#include "utility/Serialize.h"

using namespace Network;

static size_t const MAX_FILTER_SIZE = 520;

char const FilterAddMessage::COMMAND[] = "filteradd";

FilterAddMessage::FilterAddMessage(std::vector<uint8_t> const & data)
    : Message(COMMAND)
    , data_(data)
{
}

FilterAddMessage::FilterAddMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    data_ = Utility::VarArray<uint8_t>(in, size).value();
    if (data_.size() > MAX_FILTER_SIZE)
        throw InvalidMessageError();
}

void FilterAddMessage::serialize(std::vector<uint8_t> & out) const
{
    Message::serialize(data_, out);
}