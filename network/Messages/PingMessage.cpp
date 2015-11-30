#include "PingMessage.h"

#include "utility/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

char const PingMessage::COMMAND[] = "ping";

PingMessage::PingMessage(uint64_t nonce)
    : Message(COMMAND)
    , nonce_(nonce)
{
}

PingMessage::PingMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
    , nonce_(deserialize<uint64_t>(in, size))
{
}

void PingMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Utility::serialize(littleEndian(nonce_), payload);
    Message::serialize(payload, out);
}
