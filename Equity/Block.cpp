#include "Block.h"

#include "utility/Serialize.h"
#include "crypto/Sha256.h"
#include "utility/Utility.h"

using namespace Equity;

Block::Header::Header(uint8_t const * & in, size_t & size)
{
    version_ = Utility::deserialize<uint32_t>(in, size);
    previousBlock_ = Utility::deserializeArray<uint8_t>(Crypto::SHA256_HASH_SIZE, in, size);
    merkleRoot_ = Utility::deserializeArray<uint8_t>(Crypto::SHA256_HASH_SIZE, in, size);
    timestamp_ = Utility::deserialize<uint32_t>(in, size);
    target_ = Utility::deserialize<uint32_t>(in, size);
    nonce_ = Utility::deserialize<uint32_t>(in, size);
}

void Block::Header::serialize(std::vector<uint8_t> & out) const
{

}

Block::Block(uint8_t const * & in, size_t & size)
{
    header_ = Header(in, size);
    transactions_ = Utility::VarArray<Transaction>(in, size).value();
}

void Block::serialize(std::vector<uint8_t> & out) const
{
    header_.serialize(out);
    Utility::VarArray<Transaction>(transactions_).serialize(out);
}

static char const VERSION_LABEL[] = "\"version\":";
static char const PREVIOUS_LABEL[] = "\"previous\":";
static char const ROOT_LABEL[] = "\"root\":";
static char const TIME_LABEL[] = "\"time\":";
static char const TARGET_LABEL[] = "\"target\":";
static char const NONCE_LABEL[] = "\"nonce\":";
static char const TRANSACTIONS_LABEL[] = "\"transactions\":";

std::string Block::toJson() const
{
    std::string json;
    json += "{";
    json += VERSION_LABEL + std::to_string(header_.version_) + ',';
    json += PREVIOUS_LABEL + Utility::toJson(header_.previousBlock_) + ',';
    json += ROOT_LABEL + Utility::toJson(header_.merkleRoot_) + ',';
    json += TIME_LABEL + std::to_string(header_.timestamp_) + ',';
    json += TARGET_LABEL + std::to_string(header_.target_) + ',';
    json += NONCE_LABEL + std::to_string(header_.nonce_) + ',';
    json += TRANSACTIONS_LABEL + Utility::toJson(transactions_);
    json += "}";

    return json;
}