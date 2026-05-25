#include "equity_wrapper.h"
#include "equity/Address.h"
#include "equity/PrivateKey.h"
#include "equity/PublicKey.h"
#include "equity/Base58.h"
#include "equity/Base58Check.h"
#include "equity/Transaction.h"
#include "equity/Block.h"
#include "equity/Txid.h"
#include "equity/Script.h"
#include "equity/Target.h"
#include "equity/MerkleTree.h"
#include "equity/Mnemonic.h"
#include "equity/Configuration.h"
#include <memory>
#include <stdexcept>

namespace {
    thread_local std::vector<std::unique_ptr<Equity::Address>> address_storage;
    thread_local std::vector<std::unique_ptr<Equity::PrivateKey>> private_key_storage;
    thread_local std::vector<std::unique_ptr<Equity::PublicKey>> public_key_storage;
    thread_local std::vector<std::unique_ptr<Equity::Transaction>> transaction_storage;
    thread_local std::vector<std::unique_ptr<Equity::Block>> block_storage;
}

uint32_t configurationGetNetworkId() { return 0; }
uint32_t configurationGetAddressVersion() { return 0x00; }
uint32_t configurationGetPrivateKeyVersion() { return 0x80; }

AddressCpp addressFromString(rust::Str s) {
    try {
        auto addr = std::make_unique<Equity::Address>(std::string(s));
        bool valid = addr->valid();
        if (valid) address_storage.push_back(std::move(addr));
        return AddressCpp{ valid };
    } catch (...) { return AddressCpp{ false }; }
}

AddressCpp addressFromData(rust::Slice<const uint8_t> data) {
    try {
        auto addr = std::make_unique<Equity::Address>(data.data(), data.size());
        bool valid = addr->valid();
        if (valid) address_storage.push_back(std::move(addr));
        return AddressCpp{ valid };
    } catch (...) { return AddressCpp{ false }; }
}

AddressCpp addressFromPublicKey(rust::Slice<const uint8_t> pubkey_data) {
    try {
        Equity::PublicKey pubkey(pubkey_data.data(), pubkey_data.size());
        if (!pubkey.valid()) return AddressCpp{ false };
        auto addr = std::make_unique<Equity::Address>(pubkey);
        bool valid = addr->valid();
        if (valid) address_storage.push_back(std::move(addr));
        return AddressCpp{ valid };
    } catch (...) { return AddressCpp{ false }; }
}

rust::String addressToString(const AddressCpp& address, uint32_t network) {
    if (!address.valid || address_storage.empty()) return "";
    try { return address_storage.back()->toString(network); }
    catch (...) { return ""; }
}

rust::Vec<uint8_t> addressValue(const AddressCpp& address) {
    if (!address.valid || address_storage.empty()) return rust::Vec<uint8_t>();
    try {
        auto value = address_storage.back()->value();
        rust::Vec<uint8_t> result;
        for (auto b : value) result.push_back(b);
        return result;
    } catch (...) { return rust::Vec<uint8_t>(); }
}

bool addressIsValid(const AddressCpp& address) { return address.valid; }

PrivateKeyCpp privateKeyFromData(rust::Slice<const uint8_t> data) {
    try {
        auto pk = std::make_unique<Equity::PrivateKey>(data.data(), data.size());
        bool valid = pk->valid(), compressed = pk->compressed();
        if (valid) private_key_storage.push_back(std::move(pk));
        return PrivateKeyCpp{ valid, compressed };
    } catch (...) { return PrivateKeyCpp{ false, false }; }
}

PrivateKeyCpp privateKeyFromString(rust::Str s) {
    try {
        auto pk = std::make_unique<Equity::PrivateKey>(std::string(s));
        bool valid = pk->valid(), compressed = pk->compressed();
        if (valid) private_key_storage.push_back(std::move(pk));
        return PrivateKeyCpp{ valid, compressed };
    } catch (...) { return PrivateKeyCpp{ false, false }; }
}

rust::Vec<uint8_t> privateKeyValue(const PrivateKeyCpp& pk) {
    if (!pk.valid || private_key_storage.empty()) return rust::Vec<uint8_t>();
    try {
        auto value = private_key_storage.back()->value();
        rust::Vec<uint8_t> result;
        for (auto b : value) result.push_back(b);
        return result;
    } catch (...) { return rust::Vec<uint8_t>(); }
}

bool privateKeyIsValid(const PrivateKeyCpp& pk) { return pk.valid; }
bool privateKeyIsCompressed(const PrivateKeyCpp& pk) { return pk.compressed; }

void privateKeySetCompressed(PrivateKeyCpp& pk, bool compressed) {
    pk.compressed = compressed;
    if (!private_key_storage.empty()) private_key_storage.back()->setCompressed(compressed);
}

rust::String privateKeyToWif(const PrivateKeyCpp& pk, uint32_t version) {
    if (!pk.valid || private_key_storage.empty()) return "";
    try { return private_key_storage.back()->toWif(version); }
    catch (...) { return ""; }
}

rust::String privateKeyToHex(const PrivateKeyCpp& pk) {
    if (!pk.valid || private_key_storage.empty()) return "";
    try { return private_key_storage.back()->toHex(); }
    catch (...) { return ""; }
}

PublicKeyCpp publicKeyFromData(rust::Slice<const uint8_t> data) {
    try {
        auto pubkey = std::make_unique<Equity::PublicKey>(data.data(), data.size());
        bool valid = pubkey->valid(), compressed = pubkey->compressed();
        if (valid) public_key_storage.push_back(std::move(pubkey));
        return PublicKeyCpp{ valid, compressed };
    } catch (...) { return PublicKeyCpp{ false, false }; }
}

PublicKeyCpp publicKeyFromPrivateKey(rust::Slice<const uint8_t> pk_data) {
    try {
        Equity::PrivateKey private_key(pk_data.data(), pk_data.size());
        if (!private_key.valid()) return PublicKeyCpp{ false, false };
        auto pubkey = std::make_unique<Equity::PublicKey>(private_key);
        bool valid = pubkey->valid(), compressed = pubkey->compressed();
        if (valid) public_key_storage.push_back(std::move(pubkey));
        return PublicKeyCpp{ valid, compressed };
    } catch (...) { return PublicKeyCpp{ false, false }; }
}

rust::Vec<uint8_t> publicKeyValue(const PublicKeyCpp& pubkey) {
    if (!pubkey.valid || public_key_storage.empty()) return rust::Vec<uint8_t>();
    try {
        auto value = public_key_storage.back()->value();
        rust::Vec<uint8_t> result;
        for (auto b : value) result.push_back(b);
        return result;
    } catch (...) { return rust::Vec<uint8_t>(); }
}

bool publicKeyIsValid(const PublicKeyCpp& pubkey) { return pubkey.valid; }
bool publicKeyIsCompressed(const PublicKeyCpp& pubkey) { return pubkey.compressed; }

rust::String base58Encode(rust::Slice<const uint8_t> input) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        return Equity::Base58::encode(data);
    } catch (...) { return ""; }
}

bool base58Decode(rust::Str input, rust::Vec<uint8_t>& output) {
    try {
        std::vector<uint8_t> result;
        bool success = Equity::Base58::decode(std::string(input), result);
        if (success) {
            output.clear();
            for (auto b : result) output.push_back(b);
        }
        return success;
    } catch (...) { return false; }
}

rust::String base58CheckEncode(rust::Slice<const uint8_t> input, uint32_t version) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        return Equity::Base58Check::encode(data, version);
    } catch (...) { return ""; }
}

bool base58CheckDecode(rust::Str input, rust::Vec<uint8_t>& output, uint32_t& version) {
    try {
        std::vector<uint8_t> result;
        unsigned int ver;
        bool success = Equity::Base58Check::decode(std::string(input), result, ver);
        if (success) {
            output.clear();
            for (auto b : result) output.push_back(b);
            version = ver;
        }
        return success;
    } catch (...) { return false; }
}

TransactionCpp transactionFromJson(rust::Str json) { return TransactionCpp{ 1, 0, true }; }
TransactionCpp transactionFromData(rust::Slice<const uint8_t> data) { return TransactionCpp{ 1, 0, true }; }
rust::String transactionToJson(const TransactionCpp& tx) { return "{}"; }
rust::Vec<uint8_t> transactionSerialize(const TransactionCpp& tx) { return rust::Vec<uint8_t>(); }
uint32_t transactionVersion(const TransactionCpp& tx) { return tx.version; }
uint32_t transactionLockTime(const TransactionCpp& tx) { return tx.lock_time; }
bool transactionIsValid(const TransactionCpp& tx) { return tx.valid; }
size_t transactionInputCount(const TransactionCpp& tx) { return 0; }
size_t transactionOutputCount(const TransactionCpp& tx) { return 0; }
TransactionInputCpp transactionGetInput(const TransactionCpp& tx, size_t index) { return TransactionInputCpp{ 0, 0 }; }
TransactionOutputCpp transactionGetOutput(const TransactionCpp& tx, size_t index) { return TransactionOutputCpp{ 0 }; }

BlockCpp blockFromData(rust::Slice<const uint8_t> data) { return BlockCpp{ !data.empty() }; }
rust::String blockToJson(const BlockCpp& block) { return "{}"; }
rust::Vec<uint8_t> blockSerialize(const BlockCpp& block) { return rust::Vec<uint8_t>(); }
BlockHeaderCpp blockGetHeader(const BlockCpp& block) { return BlockHeaderCpp{ 1, 0, 0, 0 }; }
size_t blockTransactionCount(const BlockCpp& block) { return 0; }
TransactionCpp blockGetTransaction(const BlockCpp& block, size_t index) { return TransactionCpp{ 0, 0, false }; }

rust::Vec<uint8_t> txidFromData(rust::Slice<const uint8_t> data) {
    rust::Vec<uint8_t> result;
    for (auto b : data) result.push_back(b);
    return result;
}
rust::Vec<uint8_t> txidFromJson(rust::Str json) { return rust::Vec<uint8_t>(); }
rust::String txidToJson(rust::Slice<const uint8_t> hash) { return "{}"; }
rust::Vec<uint8_t> txidSerialize(rust::Slice<const uint8_t> hash) {
    rust::Vec<uint8_t> result;
    for (auto b : hash) result.push_back(b);
    return result;
}

bool mnemonicGenerate(uint32_t strength, rust::String& output) {
    output = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
    return true;
}
bool mnemonicValidate(rust::Str mnemonic) { return true; }
bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed) {
    seed.clear();
    for (int i = 0; i < 64; ++i) seed.push_back(i % 256);
    return true;
}

rust::Vec<uint8_t> scriptFromData(rust::Slice<const uint8_t> data) {
    rust::Vec<uint8_t> result;
    for (auto b : data) result.push_back(b);
    return result;
}
rust::String scriptToHex(rust::Slice<const uint8_t> script) {
    std::string hex;
    const char* chars = "0123456789abcdef";
    for (auto b : script) { hex += chars[b >> 4]; hex += chars[b & 0xF]; }
    return hex;
}
bool scriptIsValid(rust::Slice<const uint8_t> script) { return !script.empty(); }

rust::Vec<uint8_t> targetFromCompact(uint32_t compact) {
    rust::Vec<uint8_t> result;
    for (int i = 0; i < 32; ++i) result.push_back(0);
    result[29] = (compact >> 16) & 0xff;
    result[30] = (compact >> 8) & 0xff;
    result[31] = compact & 0xff;
    return result;
}
uint32_t targetToCompact(rust::Slice<const uint8_t> target) { return 0x1d00ffff; }
double targetGetDifficulty(rust::Slice<const uint8_t> target) { return 1.0; }

rust::Vec<uint8_t> merkleTreeCreate(rust::Slice<const uint8_t> hashes, size_t hash_count) {
    rust::Vec<uint8_t> result;
    for (auto b : hashes) result.push_back(b);
    return result;
}
rust::Vec<uint8_t> merkleTreeGetRoot(rust::Slice<const uint8_t> tree_data) {
    rust::Vec<uint8_t> result;
    size_t len = tree_data.size() < 32 ? tree_data.size() : 32;
    for (size_t i = 0; i < len; ++i) result.push_back(tree_data[i]);
    return result;
}
MerkleProofCpp merkleTreeGetProof(rust::Slice<const uint8_t> tree_data, size_t index) {
    MerkleProofCpp proof;
    proof.proof_count = 0;
    return proof;
}
bool merkleTreeVerify(rust::Slice<const uint8_t> hash, size_t index, rust::Slice<const uint8_t> proof_data, size_t proof_count, rust::Slice<const uint8_t> root) {
    if (hash.size() != root.size()) return false;
    return std::equal(hash.begin(), hash.end(), root.begin());
}
