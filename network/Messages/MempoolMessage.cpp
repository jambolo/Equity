#include "MempoolMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const MempoolMessage::TYPE[] = "mempool";

MempoolMessage::MempoolMessage()
    : Message(TYPE)
{
}

MempoolMessage::MempoolMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void MempoolMessage::serialize(std::vector<uint8_t> & out) const
{
}
