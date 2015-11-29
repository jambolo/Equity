#include "MempoolMessage.h"

using namespace Network;

char const MempoolMessage::COMMAND[] = "mempool";

MempoolMessage::MempoolMessage()
    : Message(COMMAND)
{
}

MempoolMessage::MempoolMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void Message::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
