#include "PingMessage.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

char const PingMessage::TYPE[] = "ping";

PingMessage::PingMessage(uint64_t nonce)
    : Message(TYPE)
    , nonce_(nonce)
{
}

PingMessage::PingMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    nonce_ = littleEndian(P2p::deserialize<uint64_t>(in, size));
}

void PingMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(littleEndian(nonce_), out);
}
