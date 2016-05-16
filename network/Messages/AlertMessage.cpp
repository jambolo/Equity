#include "AlertMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

// public key
// 04fc9702847840aaf195de8442ebecedf5b095cdbb9bc716bda9110971b28a49e0ead8564ff0db22209e0374782c093bb899692d524e9d6a6956e7c5ecbcd68284
// bitcoin address 1AGRxqDa5WjUKBwHB9XYEjmkv1ucoUUy1s

char const AlertMessage::TYPE[] = "alert";

AlertMessage::AlertMessage(std::vector<uint8_t> const & message)
    : Message(TYPE)
    , message_(message)
{
}

AlertMessage::AlertMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
    , message_(in, in + size)
{
}

void AlertMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(message_, out);
}
