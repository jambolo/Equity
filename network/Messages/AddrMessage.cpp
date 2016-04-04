#include "AddrMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const AddrMessage::TYPE[] = "addr";

AddrMessage::AddrMessage(std::vector<Address> const & addresses)
    : Message(TYPE)
    , addresses_(addresses)
{
}

AddrMessage::AddrMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    addresses_ = Utility::VarArray<Address>(in, size).value();
}

void AddrMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(Utility::VarArray<Address>(addresses_), out);
}
