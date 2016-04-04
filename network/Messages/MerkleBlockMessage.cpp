#include "MerkleBlockMessage.h"

#include "utility/Endian.h"
#include "utility/Serialize.h"

using namespace Network;
using namespace Utility;

char const MerkleBlockMessage::TYPE[] = "merkleblock";

MerkleBlockMessage::MerkleBlockMessage(Equity::Block::Header const &  header,
                                       uint32_t                       count,
                                       Crypto::Sha256HashList const & hashes,
                                       BitArray const &               flags)
    : Message(TYPE)
    , header_(header)
    , count_(count)
    , hashes_(hashes)
    , flags_(flags)
{
}

MerkleBlockMessage::MerkleBlockMessage(uint8_t const * & in, size_t & size)
    : Message(TYPE)
{
    header_ = Equity::Block::Header(in, size);
    count_ = littleEndian(deserialize<uint32_t>(in, size));
    hashes_ = VarArray<Crypto::Sha256Hash>(in, size).value();
    flags_ = BitArray(in, size);
}

void MerkleBlockMessage::serialize(std::vector<uint8_t> & out) const
{
    header_.serialize(out);
    Utility::serialize(littleEndian(count_), out);
    VarArray<Crypto::Sha256Hash>(hashes_).serialize(out);
    flags_.serialize(out);
}
