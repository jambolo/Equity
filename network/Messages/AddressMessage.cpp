#include "AddressMessage.h"

#include "p2p/Serialize.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace Network;

char const AddressMessage::TYPE[] = "addr";

AddressMessage::AddressMessage(std::vector<Address> const & addresses)
    : Message(TYPE)
    , addresses_(addresses)
{
}

AddressMessage::AddressMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    addresses_ = P2p::VarArray<Address>(in, size).value();
}

void AddressMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(P2p::VarArray<Address>(addresses_), out);
}

json Network::AddressMessage::toJson() const
{
    return json::object(
    {
        { "addresses", P2p::toJson(addresses_) }
    });
}
