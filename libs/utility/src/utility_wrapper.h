#pragma once

#include "rust/cxx.h"
#include <cstdint>

extern "C" {

// Endian conversion functions
uint16_t utilityEndianSwap16(uint16_t x);
uint32_t utilityEndianSwap32(uint32_t x);
uint64_t utilityEndianSwap64(uint64_t x);

uint16_t utilityEndianLittle16(uint16_t x);
uint32_t utilityEndianLittle32(uint32_t x);
uint64_t utilityEndianLittle64(uint64_t x);

uint16_t utilityEndianBig16(uint16_t x);
uint32_t utilityEndianBig32(uint32_t x);
uint64_t utilityEndianBig64(uint64_t x);

// Utility hex conversion functions
bool utilityToHex(const rust::Vec<uint8_t>& input, rust::String& output);
bool utilityToHexPtr(const uint8_t* data, size_t length, rust::String& output);
bool utilityFromHex(const rust::String& hex, rust::Vec<uint8_t>& output);
bool utilityFromHexPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output);

bool utilityToHexR(const rust::Vec<uint8_t>& input, rust::String& output);
bool utilityToHexRPtr(const uint8_t* data, size_t length, rust::String& output);
bool utilityFromHexR(const rust::String& hex, rust::Vec<uint8_t>& output);
bool utilityFromHexRPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output);

bool utilityShorten(const rust::String& input, size_t size, rust::String& output);

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

} // extern "C"
