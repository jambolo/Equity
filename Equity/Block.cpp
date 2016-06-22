#include "Block.h"

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include "utility/Utility.h"

using namespace Equity;

Block::Header::Header(uint8_t const * & in, size_t & size)
{
    version         = P2p::deserialize<uint32_t>(in, size);
    previousBlock   = P2p::deserializeArray<Crypto::Sha256Hash>(in, size);
    merkleRoot      = P2p::deserializeArray<Crypto::Sha256Hash>(in, size);
    timestamp       = P2p::deserialize<uint32_t>(in, size);
    target          = P2p::deserialize<uint32_t>(in, size);
    nonce           = P2p::deserialize<uint32_t>(in, size);
}

void Block::Header::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize((uint32_t)version, out);
    P2p::serialize(previousBlock, out);
    P2p::serialize(merkleRoot, out);
    P2p::serialize(timestamp, out);
    P2p::serialize(target, out);
    P2p::serialize(nonce, out);
}

json Block::Header::toJson() const
{
    return json::object(
    {
        { "version", version },
        { "previous", Utility::toHexR(previousBlock).c_str() }, // Hashes are stored as big-endian but displayed as little-endian
        { "root", Utility::toHexR(merkleRoot).c_str() }, // Hashes are stored as big-endian but displayed as little-endian
        { "time", timestamp },
        { "target", target },
        { "nonce", nonce }
    });
}

Block::Block(Header const & header, TransactionList const & transactions)
    : header_(header)
    , transactions_(transactions)
{
}

Block::Block(uint8_t const * & in, size_t & size)
{
    header_         = Header(in, size);
    transactions_   = P2p::VarArray<Transaction>(in, size).value();
}

void Block::serialize(std::vector<uint8_t> & out) const
{
    P2p::serialize(header_, out);
    P2p::serialize(P2p::VarArray<Transaction>(transactions_), out);
}

json Block::toJson() const
{
    return json::object(
    {
        { "header", header_.toJson() },
        { "transactions", P2p::toJson(transactions_) }
    });
}
