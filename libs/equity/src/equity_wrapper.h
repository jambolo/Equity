#pragma once

#include "rust/cxx.h"
#include <cstdint>

// CXX-compatible function declarations for Equity bridge
// These use cxx's Rust types which are compatible with the bridge

// Configuration functions
uint32_t configurationGetNetworkId();
uint32_t configurationGetAddressVersion();
uint32_t configurationGetPrivateKeyVersion();

// Mnemonic functions
bool mnemonicGenerate(uint32_t strength, rust::String& output);
bool mnemonicValidate(rust::Str mnemonic);
bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed);

// MerkleTree functions
bool utilityMerkleTreeCreate(const rust::Vec<rust::Vec<uint8_t>>& hashes, rust::Vec<uint8_t>& tree_data);
bool utilityMerkleTreeCreateFromSerialized(const rust::Vec<uint8_t>& serialized_hashes, rust::Vec<uint8_t>& tree_data);
bool utilityMerkleTreeGetRoot(const rust::Vec<uint8_t>& tree_data, rust::Vec<uint8_t>& root);
bool utilityMerkleTreeGetHashAt(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& hash);
bool utilityMerkleTreeGetProof(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<rust::Vec<uint8_t>>& proof);
bool utilityMerkleTreeGetProofSerialized(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& serialized_proof);
bool utilityMerkleTreeVerify(
    const rust::Vec<uint8_t>& hash,
    size_t index,
    const rust::Vec<rust::Vec<uint8_t>>& proof,
    const rust::Vec<uint8_t>& root
);
bool utilityMerkleTreeVerifySerialized(
    const rust::Vec<uint8_t>& hash,
    size_t index,
    const rust::Vec<uint8_t>& serialized_proof,
    const rust::Vec<uint8_t>& root
);
