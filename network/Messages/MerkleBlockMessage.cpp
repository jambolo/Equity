#include "MerkleBlockMessage.h"

#include "p2p/Serialize.h"
#include "utility/Endian.h"

using namespace Network;
using namespace Utility;

char const MerkleBlockMessage::TYPE[] = "merkleblock";

MerkleBlockMessage::MerkleBlockMessage(Equity::Block::Header const &  header,
                                       uint32_t                       count,
                                       Crypto::Sha256HashList const & hashes,
                                       P2p::BitArray const &          flags)
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
    count_ = Endian::little(P2p::deserialize<uint32_t>(in, size));
    hashes_ = P2p::VarArray<Crypto::Sha256Hash>(in, size).value();
    flags_ = P2p::BitArray(in, size);
}

void MerkleBlockMessage::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(header_, out);
    P2p::serialize(Endian::little(count_), out);
    P2p::serialize(P2p::VarArray<Crypto::Sha256Hash>(hashes_), out);
    P2p::serialize(flags_, out);
}

json MerkleBlockMessage::toJson() const
{
    return std::string("...");
}
