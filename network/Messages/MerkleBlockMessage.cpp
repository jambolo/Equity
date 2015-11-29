#include "MerkleBlockMessage.h"

using namespace Network;

char const MerkleBlockMessage::COMMAND[] = "merkleblock";

MerkleBlockMessage::MerkleBlockMessage()
    : Message(COMMAND)
{
}

MerkleBlockMessage::MerkleBlockMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
}

void Message::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    Message::serialize(payload, out);
}
