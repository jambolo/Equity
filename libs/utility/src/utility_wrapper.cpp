#include "utility_wrapper.h"
#include "Endian.h"
#include "Utility.h"
#include "MerkleTree.h"
#include <cstring>
#include <stdexcept>

extern "C" {

// Endian conversion functions
uint16_t utilityEndianSwap16(uint16_t x) {
    return Utility::Endian::swap(x);
}

uint32_t utilityEndianSwap32(uint32_t x) {
    return Utility::Endian::swap(x);
}

uint64_t utilityEndianSwap64(uint64_t x) {
    return Utility::Endian::swap(x);
}

uint16_t utilityEndianLittle16(uint16_t x) {
    return Utility::Endian::little(x);
}

uint32_t utilityEndianLittle32(uint32_t x) {
    return Utility::Endian::little(x);
}

uint64_t utilityEndianLittle64(uint64_t x) {
    return Utility::Endian::little(x);
}

uint16_t utilityEndianBig16(uint16_t x) {
    return Utility::Endian::big(x);
}

uint32_t utilityEndianBig32(uint32_t x) {
    return Utility::Endian::big(x);
}

uint64_t utilityEndianBig64(uint64_t x) {
    return Utility::Endian::big(x);
}

// Utility hex conversion functions
bool utilityToHex(const rust::Vec<uint8_t>& input, rust::String& output) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        std::string result = Utility::toHex(data);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexPtr(const uint8_t* data, size_t length, rust::String& output) {
    try {
        if (!data && length > 0) {
            return false;
        }
        std::string result = Utility::toHex(data, length);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHex(const rust::String& hex, rust::Vec<uint8_t>& output) {
    try {
        std::string hex_str(hex);
        std::vector<uint8_t> result = Utility::fromHex(hex_str);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output) {
    try {
        if (!hex && length > 0) {
            return false;
        }
        std::vector<uint8_t> result = Utility::fromHex(reinterpret_cast<const char*>(hex), length);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexR(const rust::Vec<uint8_t>& input, rust::String& output) {
    try {
        std::vector<uint8_t> data(input.begin(), input.end());
        std::string result = Utility::toHexR(data);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityToHexRPtr(const uint8_t* data, size_t length, rust::String& output) {
    try {
        if (!data && length > 0) {
            return false;
        }
        std::string result = Utility::toHexR(data, length);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexR(const rust::String& hex, rust::Vec<uint8_t>& output) {
    try {
        std::string hex_str(hex);
        std::vector<uint8_t> result = Utility::fromHexR(hex_str);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityFromHexRPtr(const int8_t* hex, size_t length, rust::Vec<uint8_t>& output) {
    try {
        if (!hex && length > 0) {
            return false;
        }
        std::vector<uint8_t> result = Utility::fromHexR(reinterpret_cast<const char*>(hex), length);
        
        output.clear();
        for (uint8_t byte : result) {
            output.push_back(byte);
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityShorten(const rust::String& input, size_t size, rust::String& output) {
    try {
        std::string input_str(input);
        std::string result = Utility::shorten(input_str, size);
        output = result;
        return true;
    } catch (...) {
        return false;
    }
}

// MerkleTree functions - using serialized format to avoid Vec<Vec<u8>> issues
bool utilityMerkleTreeCreateFromSerialized(const rust::Vec<uint8_t>& serialized_hashes, rust::Vec<uint8_t>& tree_data) {
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
        Utility::MerkleTree tree(hashes);
        
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
    } catch (...) {
        return false;
    }
}

bool utilityMerkleTreeCreate(const rust::Vec<rust::Vec<uint8_t>>& hashes, rust::Vec<uint8_t>& tree_data) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Instead, use the serialized version
    return false; // Use utilityMerkleTreeCreateFromSerialized instead
}

bool utilityMerkleTreeGetRoot(const rust::Vec<uint8_t>& tree_data, rust::Vec<uint8_t>& root) {
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
        
        Utility::MerkleTree tree(hashes);
        Crypto::Sha256Hash tree_root = tree.root();
        
        root.clear();
        for (uint8_t byte : tree_root) {
            root.push_back(byte);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityMerkleTreeGetHashAt(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& hash) {
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
        
        Utility::MerkleTree tree(hashes);
        Crypto::Sha256Hash result_hash = tree.hashAt(index);
        
        hash.clear();
        for (uint8_t byte : result_hash) {
            hash.push_back(byte);
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool utilityMerkleTreeGetProofSerialized(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<uint8_t>& serialized_proof) {
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
        
        Utility::MerkleTree tree(hashes);
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
    } catch (...) {
        return false;
    }
}

bool utilityMerkleTreeGetProof(const rust::Vec<uint8_t>& tree_data, size_t index, rust::Vec<rust::Vec<uint8_t>>& proof) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Use the serialized version instead
    return false; // Use utilityMerkleTreeGetProofSerialized instead
}

bool utilityMerkleTreeVerifySerialized(
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
        
        return Utility::MerkleTree::verify(input_hash, index, proof, merkle_root);
    } catch (...) {
        return false;
    }
}

bool utilityMerkleTreeVerify(
    const rust::Vec<uint8_t>& hash,
    size_t index,
    const rust::Vec<rust::Vec<uint8_t>>& proof,
    const rust::Vec<uint8_t>& root
) {
    // This would require Vec<Vec<u8>> which cxx doesn't support well
    // Use the serialized version instead
    return false; // Use utilityMerkleTreeVerifySerialized instead
}

} // extern "C"
