#include "PongMessage.h"

#include "utility/Serialize.h"
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
    , nonce_(deserialize<uint64_t>(in, size))
{
}

void PongMessage::serialize(std::vector<uint8_t> & out) const
{
    Utility::serialize(littleEndian(nonce_), out);
}
