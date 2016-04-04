#include "PongMessage.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

char const PongMessage::TYPE[] = "pong";

PongMessage::PongMessage(uint64_t nonce)
    : Message(TYPE)
    , nonce_(nonce)
{
}

PongMessage::PongMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    nonce_ = littleEndian(P2p::deserialize<uint64_t>(in, size));
}

void PongMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(littleEndian(nonce_), out);
}
