#include "TransactionMessage.h"

using namespace Network;

char const TransactionMessage::TYPE[] = "tx";

TransactionMessage::TransactionMessage()
    : Message(TYPE)
{
}

TransactionMessage::TransactionMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void TransactionMessage::serialize(std::vector<uint8_t> & out) const
{
}
