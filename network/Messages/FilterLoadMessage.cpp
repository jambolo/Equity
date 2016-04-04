#include "FilterLoadMessage.h"

#include "utility/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

static int const    MAX_NUM_HASH_FUNCIONS   = 50;       // Maximum number of hash functions allowed in a message
static size_t const MAX_FILTER_SIZE         = 36000;    // Maximum size of a filter in a message

char const FilterLoadMessage::COMMAND[] = "filterload";

FilterLoadMessage::FilterLoadMessage(std::vector<uint8_t> const & filter,
                                     uint32_t                     nHashFuncs,
                                     uint32_t                     tweak,
                                     uint8_t                      flags)
    : Message(COMMAND)
    , filter_(filter)
    , nHashFuncs_(nHashFuncs)
    , tweak_(tweak)
    , flags_(flags)
{
}

FilterLoadMessage::FilterLoadMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    filter_ = VarArray<uint8_t>(in, size).value();
    if (filter_.size() > MAX_FILTER_SIZE)
        throw InvalidMessageError();
    nHashFuncs_ = littleEndian(deserialize<uint32_t>(in, size));
    if (nHashFuncs_ > MAX_NUM_HASH_FUNCIONS)
        throw InvalidMessageError();
    tweak_ = littleEndian(deserialize<uint32_t>(in, size));
    flags_ = deserialize<uint8_t>(in, size);
}

void FilterLoadMessage::serialize(std::vector<uint8_t> & out) const
{
    VarArray<uint8_t>(filter_).serialize(out);
    Utility::serialize(littleEndian(nHashFuncs_), out);
    Utility::serialize(littleEndian(tweak_), out);
    Utility::serialize(flags_, out);
}
