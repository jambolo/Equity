#include "TxMessage.h"

#include "utility/Serialize.h"

using namespace Network;

char const TxMessage::COMMAND[] = "tx";

TxMessage::TxMessage()
    : Message(COMMAND)
{
}

TxMessage::TxMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void TxMessage::serialize(std::vector<uint8_t> & out) const
{
}
