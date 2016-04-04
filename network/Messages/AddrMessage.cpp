#include "AddrMessage.h"

#include "p2p/Serialize.h"

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
    addresses_ = P2p::VarArray<Address>(in, size).value();
}

void AddrMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(P2p::VarArray<Address>(addresses_), out);
}
