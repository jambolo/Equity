#include "AddrMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const AddrMessage::COMMAND[] = "addr";

AddrMessage::AddrMessage(std::vector<Address> const & addresses)
    : Message(COMMAND)
    , addresses_(addresses)
{
}

AddrMessage::AddrMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    addresses_ = Utility::VarArray<Address>(in, size).value();
}

void AddrMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Utility::VarArray<Address>(addresses_).serialize(payload);
    Message::serialize(payload, out);
}
