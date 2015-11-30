#include "MerkleBlockMessage.h"

#include "utility/Endian.h"
#include "utility/Serialize.h"

using namespace Network;
using namespace Utility;

char const MerkleBlockMessage::COMMAND[] = "merkleblock";

MerkleBlockMessage::MerkleBlockMessage(Equity::Block::Header const &  header,
                                       uint32_t                       count,
                                       Crypto::Sha256HashList const & hashes,
                                       BitArray const &               flags)
    : Message(COMMAND)
    , header_(header)
    , count_(count)
    , hashes_(hashes)
    , flags_(flags)
{
}

MerkleBlockMessage::MerkleBlockMessage(uint8_t const * & in, size_t & size)
    : Message(COMMAND)
{
    header_ = Equity::Block::Header(in, size);
    count_ = littleEndian(deserialize<uint32_t>(in, size));
    hashes_ = VarArray<Crypto::Sha256Hash>(in, size).value();
    flags_ = BitArray(in, size);
}

void MerkleBlockMessage::serialize(std::vector<uint8_t> & out) const
{
    std::vector<uint8_t> payload;
    header_.serialize(payload);
    Utility::serialize(littleEndian(count_), payload);
    VarArray<Crypto::Sha256Hash>(hashes_).serialize(payload);
    flags_.serialize(payload);
    Message::serialize(payload, out);
}
