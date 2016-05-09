#include "RequestTransactionsMessage.h"

#include "p2p/Serialize.h"

using namespace Network;

char const RequestTransactionsMessage::TYPE[] = "mempool";

RequestTransactionsMessage::RequestTransactionsMessage()
    : Message(TYPE)
{
}

RequestTransactionsMessage::RequestTransactionsMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
}

void RequestTransactionsMessage::serialize(std::vector<uint8_t> & out) const
{
}
