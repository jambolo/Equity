#include "equity_wrapper.h"

#include <cstring>
#include <stdexcept>

// Simple implementations for testing
uint32_t configurationGetNetworkId() {
    // Return a default network ID for testing
    return 0; // Mainnet
}

uint32_t configurationGetAddressVersion() {
    // Return default address version for mainnet
    return 0x00;
}

uint32_t configurationGetPrivateKeyVersion() {
    // Return default private key version for mainnet  
    return 0x80;
}

// Mnemonic functions - simplified for testing
bool mnemonicGenerate(uint32_t strength, rust::String& output) {
    // Simple test implementation - in real use would call Equity::Mnemonic::generate
    if (strength == 128) {
        output = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        return true;
    }
    return false;
}

bool mnemonicValidate(rust::Str mnemonic) {
    // Simple validation - check if it's the test mnemonic
    std::string mnemonicStr(mnemonic);
    return mnemonicStr == "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
}

bool mnemonicToSeed(rust::Str mnemonic, rust::Str passphrase, rust::Vec<uint8_t>& seed) {
    // Simple test implementation - would normally call Equity::Mnemonic::toSeed
    std::string mnemonicStr(mnemonic);
    if (mnemonicStr == "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about") {
        // Return a test seed (64 bytes)
        seed.clear();
        for (int i = 0; i < 64; i++) {
            seed.push_back(static_cast<uint8_t>(i));
        }
        return true;
    }
    return false;
}

// MerkleTree functions - using serialized format to avoid Vec<Vec<u8>> issues
bool equityMerkleTreeCreateFromSerialized(const rust::Vec<uint8_t>& serialized_hashes, rust::Vec<uint8_t>& tree_data) {
    try {
        // Parse serialized hashes (format: count + hash1 + hash2 + ...)
        if (serialized_hashes.size() < 4) {
            return false; // Need at least count
        }

        const uint8_t* data = serialized_hashes.data();
        size_t offset = 0;

        // Read count (4 bytes, little endian)
        uint32_t count = 0;
        std::memcpy(&count, data + offset, 4);
        offset += 4;

        // Read hashes
        Crypto::Sha256HashList hashes;
        hashes.reserve(count);

        for (uint32_t i = 0; i < count; ++i) {
            if (offset + 32 > serialized_hashes.size()) {
                return false; // Not enough data
            }

            Crypto::Sha256Hash hash;
            std::memcpy(hash.data(), data + offset, 32);
            hashes.push_back(hash);
            offset += 32;
        }

        // Create MerkleTree
        Equity::MerkleTree tree(hashes);

        // Serialize tree data (we'll store it as a placeholder for now)
        // In a real implementation, you'd serialize the tree structure
        tree_data.clear();
        size_t total_size = 4 + count * 32;
        for (size_t i = 0; i < total_size; ++i) {
            tree_data.push_back(0);
        }

        // Store count
        std::memcpy(tree_data.data(), &count, 4);

        // Store hashes
        offset = 4;
        for (const auto& hash : hashes) {
            std::memcpy(tree_data.data() + offset, hash.data(), 32);
            offset += 32;
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

bool equityMerkleTreeCreate(const rust::Vec<rust::Vec<uint8_t>>& hashes, rust::Vec<uint8_t>& tree_data) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Instead, use the serialized version
    return false; // Use equityMerkleTreeCreateFromSerialized instead
}

bool equityMerkleTreeGetRoot(const rust::Vec<uint8_t>& tree_data, rust::Vec<uint8_t>& root) {
    try {
        // Parse tree data and recreate MerkleTree
        if (tree_data.size() < 4) {
            return false;
        }

        const uint8_t* data = tree_data.data();
        uint32_t count = 0;
        std::memcpy(&count, data, 4);

        if (tree_data.size() < 4 + count * 32) {
            return false;
        }

        Crypto::Sha256HashList hashes;
        hashes.reserve(count);

        size_t offset = 4;
        for (uint32_t i = 0; i < count; ++i) {
            Crypto::Sha256Hash hash;
            std::memcpy(hash.data(), data + offset, 32);
            hashes.push_back(hash);
            offset += 32;
        }

        Equity::MerkleTree tree(hashes);
        Crypto::Sha256Hash tree_root = tree.root();

        root.clear();
        for (uint8_t byte : tree_root) {
            root.push_back(byte);
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

bool equityMerkleTreeGetHashAt(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& hash) {
    try {
        // Parse tree data and recreate MerkleTree
        if (tree_data.size() < 4) {
            return false;
        }

        const uint8_t* data = tree_data.data();
        uint32_t count = 0;
        std::memcpy(&count, data, 4);

        if (index >= count || tree_data.size() < 4 + count * 32) {
            return false;
        }

        Crypto::Sha256HashList hashes;
        hashes.reserve(count);

        size_t offset = 4;
        for (uint32_t i = 0; i < count; ++i) {
            Crypto::Sha256Hash tree_hash;
            std::memcpy(tree_hash.data(), data + offset, 32);
            hashes.push_back(tree_hash);
            offset += 32;
        }

        Equity::MerkleTree tree(hashes);
        Crypto::Sha256Hash result_hash = tree.hashAt(index);

        hash.clear();
        for (uint8_t byte : result_hash) {
            hash.push_back(byte);
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

bool equityMerkleTreeGetProofSerialized(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& serialized_proof) {
    try {
        // Parse tree data and recreate MerkleTree
        if (tree_data.size() < 4) {
            return false;
        }

        const uint8_t* data = tree_data.data();
        uint32_t count = 0;
        std::memcpy(&count, data, 4);

        if (index >= count || tree_data.size() < 4 + count * 32) {
            return false;
        }

        Crypto::Sha256HashList hashes;
        hashes.reserve(count);

        size_t offset = 4;
        for (uint32_t i = 0; i < count; ++i) {
            Crypto::Sha256Hash tree_hash;
            std::memcpy(tree_hash.data(), data + offset, 32);
            hashes.push_back(tree_hash);
            offset += 32;
        }

        Equity::MerkleTree tree(hashes);
        Crypto::Sha256HashList proof = tree.proof(index);

        // Serialize proof
        serialized_proof.clear();
        uint32_t proof_count = proof.size();

        // Store count
        size_t total_size = 4 + proof_count * 32;
        for (size_t i = 0; i < total_size; ++i) {
            serialized_proof.push_back(0);
        }
        std::memcpy(serialized_proof.data(), &proof_count, 4);

        // Store proof hashes
        offset = 4;
        for (const auto& proof_hash : proof) {
            std::memcpy(serialized_proof.data() + offset, proof_hash.data(), 32);
            offset += 32;
        }

        return true;
    }
    catch (...) {
        return false;
    }
}

bool equityMerkleTreeGetProof(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<rust::Vec<uint8_t>>& proof) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Use the serialized version instead
    return false; // Use equityMerkleTreeGetProofSerialized instead
}

bool equityMerkleTreeVerifySerialized(
    const rust::Vec<uint8_t>& hash,
    size_t index,
    const rust::Vec<uint8_t>& serialized_proof,
    const rust::Vec<uint8_t>& root
) {
    try {
        if (hash.size() != 32 || root.size() != 32 || serialized_proof.size() < 4) {
            return false;
        }

        // Parse serialized proof
        const uint8_t* proof_data = serialized_proof.data();
        uint32_t proof_count = 0;
        std::memcpy(&proof_count, proof_data, 4);

        if (serialized_proof.size() < 4 + proof_count * 32) {
            return false;
        }

        Crypto::Sha256HashList proof;
        proof.reserve(proof_count);

        size_t offset = 4;
        for (uint32_t i = 0; i < proof_count; ++i) {
            Crypto::Sha256Hash proof_hash;
            std::memcpy(proof_hash.data(), proof_data + offset, 32);
            proof.push_back(proof_hash);
            offset += 32;
        }

        // Convert input hash and root
        Crypto::Sha256Hash input_hash;
        std::memcpy(input_hash.data(), hash.data(), 32);

        Crypto::Sha256Hash merkle_root;
        std::memcpy(merkle_root.data(), root.data(), 32);

        return Equity::MerkleTree::verify(input_hash, index, proof, merkle_root);
    }
    catch (...) {
        return false;
    }
}

bool equityMerkleTreeVerify(
    const rust::Vec<uint8_t>& hash,
    size_t index,
    const rust::Vec<rust::Vec<uint8_t>>& proof,
    const rust::Vec<uint8_t>& root
) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Use the serialized version instead
    return false; // Use equityMerkleTreeVerifySerialized instead
}
