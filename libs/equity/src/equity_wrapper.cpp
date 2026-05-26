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
#include <random>
#include <sstream>
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

namespace {

Equity::Mnemonic::WordList splitWords(std::string const & s) {
    Equity::Mnemonic::WordList out;
    std::istringstream is(s);
    std::string w;
    while (is >> w) out.push_back(std::move(w));
    return out;
}

} // namespace

bool mnemonicGenerate(uint32_t strength, rust::String& output) {
    try {
        // BIP-39: strength is in bits; entropy size is strength/8 (must be multiple of 4).
        if (strength % 8 != 0) return false;
        size_t bytes = strength / 8;
        if (bytes < 16 || bytes > 32 || (bytes % 4) != 0) return false;

        std::vector<uint8_t> entropy(bytes);
        std::random_device rd;
        for (size_t i = 0; i < bytes; i += sizeof(uint32_t)) {
            uint32_t v = rd();
            size_t n = std::min(sizeof(uint32_t), bytes - i);
            std::memcpy(entropy.data() + i, &v, n);
        }

        Equity::Mnemonic m(entropy);
        if (!m.isValid()) return false;
        output = rust::String(m.sentence());
        return true;
    } catch (...) { return false; }
}

bool mnemonicValidate(rust::Str mnemonic) {
    try {
        Equity::Mnemonic m(splitWords(std::string(mnemonic)));
        return m.isValid();
    } catch (...) { return false; }
}

bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed) {
    try {
        Equity::Mnemonic m(splitWords(std::string(mnemonic)));
        if (!m.isValid()) return false;
        std::string pw(passphrase);
        std::vector<uint8_t> result = m.seed(pw.c_str());
        seed.clear();
        seed.reserve(result.size());
        for (uint8_t b : result) seed.push_back(b);
        return true;
    } catch (...) { return false; }
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

namespace {

constexpr size_t MERKLE_HASH_SIZE = 32;

// `tree_data` stores the leaf hashes verbatim, concatenated. Each GetRoot/GetProof
// call reconstructs an Equity::MerkleTree from those leaves.
Crypto::Sha256HashList parseMerkleLeaves(rust::Slice<const uint8_t> tree_data) {
    Crypto::Sha256HashList leaves;
    size_t n = tree_data.size() / MERKLE_HASH_SIZE;
    leaves.reserve(n);
    for (size_t i = 0; i < n; ++i) {
        Crypto::Sha256Hash h{};
        for (size_t j = 0; j < MERKLE_HASH_SIZE; ++j) {
            h[j] = tree_data[i * MERKLE_HASH_SIZE + j];
        }
        leaves.push_back(h);
    }
    return leaves;
}

rust::Vec<uint8_t> hashToRustVec(Crypto::Sha256Hash const & h) {
    rust::Vec<uint8_t> out;
    out.reserve(MERKLE_HASH_SIZE);
    for (uint8_t b : h) out.push_back(b);
    return out;
}

} // namespace

rust::Vec<uint8_t> merkleTreeCreate(rust::Slice<const uint8_t> hashes, size_t /*hash_count*/) {
    // Opaque blob: just the leaves, concatenated.
    rust::Vec<uint8_t> result;
    result.reserve(hashes.size());
    for (auto b : hashes) result.push_back(b);
    return result;
}

rust::Vec<uint8_t> merkleTreeGetRoot(rust::Slice<const uint8_t> tree_data) {
    try {
        auto leaves = parseMerkleLeaves(tree_data);
        if (leaves.empty()) return rust::Vec<uint8_t>();
        Equity::MerkleTree tree(leaves);
        return hashToRustVec(tree.root());
    } catch (...) { return rust::Vec<uint8_t>(); }
}

MerkleProofCpp merkleTreeGetProof(rust::Slice<const uint8_t> tree_data, size_t index) {
    MerkleProofCpp out;
    out.proof_count = 0;
    try {
        auto leaves = parseMerkleLeaves(tree_data);
        if (index >= leaves.size()) return out;
        Equity::MerkleTree tree(leaves);
        auto proof = tree.proof(index);
        out.proof_count = proof.size();
        out.proof_hashes.reserve(proof.size() * MERKLE_HASH_SIZE);
        for (auto const & h : proof) {
            for (uint8_t b : h) out.proof_hashes.push_back(b);
        }
    } catch (...) {}
    return out;
}

bool merkleTreeVerify(rust::Slice<const uint8_t> hash,
                      size_t                     index,
                      rust::Slice<const uint8_t> proof_data,
                      size_t                     proof_count,
                      rust::Slice<const uint8_t> root) {
    try {
        if (hash.size() != MERKLE_HASH_SIZE || root.size() != MERKLE_HASH_SIZE) return false;
        if (proof_data.size() < proof_count * MERKLE_HASH_SIZE) return false;

        Crypto::Sha256Hash h{}, r{};
        for (size_t i = 0; i < MERKLE_HASH_SIZE; ++i) { h[i] = hash[i]; r[i] = root[i]; }

        Crypto::Sha256HashList proof;
        proof.reserve(proof_count);
        for (size_t k = 0; k < proof_count; ++k) {
            Crypto::Sha256Hash ph{};
            for (size_t j = 0; j < MERKLE_HASH_SIZE; ++j) {
                ph[j] = proof_data[k * MERKLE_HASH_SIZE + j];
            }
            proof.push_back(ph);
        }
        return Equity::MerkleTree::verify(h, index, proof, r);
    } catch (...) { return false; }
}
