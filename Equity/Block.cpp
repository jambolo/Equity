#include "Block.h"

#include "crypto/Sha256.h"
#include "p2p/Serialize.h"
#include "utility/Utility.h"

using namespace Equity;

static char const HEADER_LABEL[]        = "\"header\":";
static char const VERSION_LABEL[]       = "\"version\":";
static char const PREVIOUS_LABEL[]      = "\"previous\":";
static char const ROOT_LABEL[]          = "\"root\":";
static char const TIME_LABEL[]          = "\"time\":";
static char const TARGET_LABEL[]        = "\"target\":";
static char const NONCE_LABEL[]         = "\"nonce\":";
static char const TRANSACTIONS_LABEL[]  = "\"transactions\":";

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
    P2p::serializeArray(previousBlock, out);
    P2p::serializeArray(merkleRoot, out);
    P2p::serialize(timestamp, out);
    P2p::serialize(target, out);
    P2p::serialize(nonce, out);
}

std::string Block::Header::toJson() const
{
    std::string json;
    json += "{";
    json += VERSION_LABEL + std::to_string(version) + ',';
    json += PREVIOUS_LABEL + Utility::toJson(previousBlock) + ',';
    json += ROOT_LABEL + Utility::toJson(merkleRoot) + ',';
    json += TIME_LABEL + std::to_string(timestamp) + ',';
    json += TARGET_LABEL + std::to_string(target) + ',';
    json += NONCE_LABEL + std::to_string(nonce) + ',';
    json += "}";

    return json;
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

std::string Block::toJson() const
{
    std::string json;
    json += "{";
    json += VERSION_LABEL + std::to_string(header_.version) + ',';
    json += PREVIOUS_LABEL + Utility::toJson(header_.previousBlock) + ',';
    json += ROOT_LABEL + Utility::toJson(header_.merkleRoot) + ',';
    json += TIME_LABEL + std::to_string(header_.timestamp) + ',';
    json += TARGET_LABEL + std::to_string(header_.target) + ',';
    json += NONCE_LABEL + std::to_string(header_.nonce) + ',';
    json += TRANSACTIONS_LABEL + Utility::toJson(transactions_);
    json += "}";

    return json;
}
