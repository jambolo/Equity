#pragma once

#include "rust/cxx.h"
#include <cstdint>

// Forward declarations for Equity types
namespace Equity {
    class Address;
    class PrivateKey;
    class PublicKey;
    class Transaction;
    class Block;
    struct Txid;
    class Script;
}

// CXX-compatible function declarations for Equity bridge

// Configuration functions
uint32_t configurationGetNetworkId();
uint32_t configurationGetAddressVersion();
uint32_t configurationGetPrivateKeyVersion();

// Address functions
struct AddressCpp {
    bool valid;
};
AddressCpp addressFromString(rust::Str s);
AddressCpp addressFromData(rust::Slice<const uint8_t> data);
AddressCpp addressFromPublicKey(rust::Slice<const uint8_t> pubkey_data);
rust::String addressToString(const AddressCpp& address, uint32_t network);
rust::Vec<uint8_t> addressValue(const AddressCpp& address);
bool addressIsValid(const AddressCpp& address);

// PrivateKey functions
struct PrivateKeyCpp {
    bool valid;
    bool compressed;
};
PrivateKeyCpp privateKeyFromData(rust::Slice<const uint8_t> data);
PrivateKeyCpp privateKeyFromString(rust::Str s);
rust::Vec<uint8_t> privateKeyValue(const PrivateKeyCpp& pk);
bool privateKeyIsValid(const PrivateKeyCpp& pk);
bool privateKeyIsCompressed(const PrivateKeyCpp& pk);
void privateKeySetCompressed(PrivateKeyCpp& pk, bool compressed);
rust::String privateKeyToWif(const PrivateKeyCpp& pk, uint32_t version);
rust::String privateKeyToHex(const PrivateKeyCpp& pk);

// PublicKey functions
struct PublicKeyCpp {
    bool valid;
    bool compressed;
};
PublicKeyCpp publicKeyFromData(rust::Slice<const uint8_t> data);
PublicKeyCpp publicKeyFromPrivateKey(rust::Slice<const uint8_t> pk_data);
rust::Vec<uint8_t> publicKeyValue(const PublicKeyCpp& pubkey);
bool publicKeyIsValid(const PublicKeyCpp& pubkey);
bool publicKeyIsCompressed(const PublicKeyCpp& pubkey);

// Base58 functions
rust::String base58Encode(rust::Slice<const uint8_t> input);
bool base58Decode(rust::Str input, rust::Vec<uint8_t>& output);

// Base58Check functions
rust::String base58CheckEncode(rust::Slice<const uint8_t> input, uint32_t version);
bool base58CheckDecode(rust::Str input, rust::Vec<uint8_t>& output, uint32_t& version);

// Transaction functions
struct TransactionInputCpp {
    uint32_t output_index;
    uint32_t sequence;
};

struct TransactionOutputCpp {
    uint64_t value;
};

struct TransactionCpp {
    uint32_t version;
    uint32_t lock_time;
    bool valid;
};

TransactionCpp transactionFromJson(rust::Str json);
TransactionCpp transactionFromData(rust::Slice<const uint8_t> data);
rust::String transactionToJson(const TransactionCpp& tx);
rust::Vec<uint8_t> transactionSerialize(const TransactionCpp& tx);
uint32_t transactionVersion(const TransactionCpp& tx);
uint32_t transactionLockTime(const TransactionCpp& tx);
bool transactionIsValid(const TransactionCpp& tx);
size_t transactionInputCount(const TransactionCpp& tx);
size_t transactionOutputCount(const TransactionCpp& tx);
TransactionInputCpp transactionGetInput(const TransactionCpp& tx, size_t index);
TransactionOutputCpp transactionGetOutput(const TransactionCpp& tx, size_t index);

// Block functions
struct BlockHeaderCpp {
    int32_t version;
    uint32_t timestamp;
    uint32_t target;
    uint32_t nonce;
};

struct BlockCpp {
    // Block implementation details
};

BlockCpp blockFromData(rust::Slice<const uint8_t> data);
rust::String blockToJson(const BlockCpp& block);
rust::Vec<uint8_t> blockSerialize(const BlockCpp& block);
BlockHeaderCpp blockGetHeader(const BlockCpp& block);
size_t blockTransactionCount(const BlockCpp& block);
TransactionCpp blockGetTransaction(const BlockCpp& block, size_t index);

// Txid functions
rust::Vec<uint8_t> txidFromData(rust::Slice<const uint8_t> data);
rust::Vec<uint8_t> txidFromJson(rust::Str json);
rust::String txidToJson(rust::Slice<const uint8_t> hash);
rust::Vec<uint8_t> txidSerialize(rust::Slice<const uint8_t> hash);

// Mnemonic functions
bool mnemonicGenerate(uint32_t strength, rust::String& output);
bool mnemonicValidate(rust::Str mnemonic);
bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed);

// Script functions
rust::Vec<uint8_t> scriptFromData(rust::Slice<const uint8_t> data);
rust::String scriptToHex(rust::Slice<const uint8_t> script);
bool scriptIsValid(rust::Slice<const uint8_t> script);

// Target functions
rust::Vec<uint8_t> targetFromCompact(uint32_t compact);
uint32_t targetToCompact(rust::Slice<const uint8_t> target);
double targetGetDifficulty(rust::Slice<const uint8_t> target);

// MerkleTree functions
rust::Vec<uint8_t> merkleTreeCreate(rust::Slice<const rust::Vec<uint8_t>> hashes);
rust::Vec<uint8_t> merkleTreeGetRoot(rust::Slice<const uint8_t> tree_data);
rust::Vec<rust::Vec<uint8_t>> merkleTreeGetProof(rust::Slice<const uint8_t> tree_data, size_t index);
bool merkleTreeVerify(rust::Slice<const uint8_t> hash, size_t index, rust::Slice<const rust::Vec<uint8_t>> proof, rust::Slice<const uint8_t> root);
