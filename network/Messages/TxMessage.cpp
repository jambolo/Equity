#include "TxMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const TxMessage::TYPE[] = "tx";

TxMessage::TxMessage()
    : Message(TYPE)
{
}

TxMessage::TxMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void TxMessage::serialize(std::vector<uint8_t> & out) const
{
}
