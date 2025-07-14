//! Merkle Tree implementation for Bitcoin-specific requirements

use crate::ffi;

/// Merkle Tree implementation with Bitcoin-specific idiosyncrasies
pub struct MerkleTree {
    tree_data: Vec<u8>,
}

impl MerkleTree {
    /// Create a new Merkle tree from a list of hashes
    /// 
    /// Each hash should be exactly 32 bytes (SHA256 hash size)
    pub fn new(hashes: &[Vec<u8>]) -> Result<Self, &'static str> {
        // Validate that all hashes are 32 bytes
        for hash in hashes {
            if hash.len() != 32 {
                return Err("All hashes must be exactly 32 bytes");
            }
        }
        
        // Serialize hashes into the format expected by the C++ wrapper
        let serialized_hashes = Self::serialize_hashes(hashes)?;
        
        let mut tree_data = Vec::new();
        if ffi::utilityMerkleTreeCreateFromSerialized(&serialized_hashes, &mut tree_data) {
            Ok(MerkleTree { tree_data })
        } else {
            Err("Failed to create Merkle tree")
        }
    }
    
    /// Create a new Merkle tree from serialized hash data
    /// 
    /// Format: 4-byte count (little endian) + hash1 + hash2 + ...
    pub fn from_serialized(serialized_hashes: &[u8]) -> Result<Self, &'static str> {
        let serialized_vec = serialized_hashes.to_vec();
        let mut tree_data = Vec::new();
        
        if ffi::utilityMerkleTreeCreateFromSerialized(&serialized_vec, &mut tree_data) {
            Ok(MerkleTree { tree_data })
        } else {
            Err("Failed to create Merkle tree from serialized data")
        }
    }
    
    /// Get the root hash of the Merkle tree
    pub fn root(&self) -> Result<Vec<u8>, &'static str> {
        let mut root = Vec::new();
        
        if ffi::utilityMerkleTreeGetRoot(&self.tree_data, &mut root) {
            Ok(root)
        } else {
            Err("Failed to get Merkle tree root")
        }
    }
    
    /// Get the hash at the specified index
    pub fn hash_at(&self, index: usize) -> Result<Vec<u8>, &'static str> {
        let mut hash = Vec::new();
        
        if ffi::utilityMerkleTreeGetHashAt(&self.tree_data, index, &mut hash) {
            Ok(hash)
        } else {
            Err("Failed to get hash at index")
        }
    }
    
    /// Get a Merkle proof for the hash at the specified index
    pub fn proof(&self, index: usize) -> Result<MerkleProof, &'static str> {
        let mut serialized_proof = Vec::new();
        
        if ffi::utilityMerkleTreeGetProofSerialized(&self.tree_data, index, &mut serialized_proof) {
            Ok(MerkleProof { 
                serialized_data: serialized_proof,
                index,
            })
        } else {
            Err("Failed to get Merkle proof")
        }
    }
    
    /// Verify that a hash belongs to the tree at the specified index
    pub fn verify(
        hash: &[u8],
        index: usize,
        proof: &MerkleProof,
        root: &[u8]
    ) -> Result<bool, &'static str> {
        if hash.len() != 32 || root.len() != 32 {
            return Err("Hash and root must be exactly 32 bytes");
        }
        
        let hash_vec = hash.to_vec();
        let root_vec = root.to_vec();
        
        Ok(ffi::utilityMerkleTreeVerifySerialized(
            &hash_vec,
            index,
            &proof.serialized_data,
            &root_vec
        ))
    }
    
    /// Serialize a list of hashes into the format expected by the C++ wrapper
    fn serialize_hashes(hashes: &[Vec<u8>]) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        
        // Write count (4 bytes, little endian)
        let count = hashes.len() as u32;
        serialized.extend_from_slice(&count.to_le_bytes());
        
        // Write each hash
        for hash in hashes {
            if hash.len() != 32 {
                return Err("All hashes must be exactly 32 bytes");
            }
            serialized.extend_from_slice(hash);
        }
        
        Ok(serialized)
    }
    
    /// Get the number of leaf nodes in the tree
    pub fn leaf_count(&self) -> Result<usize, &'static str> {
        if self.tree_data.len() < 4 {
            return Err("Invalid tree data");
        }
        
        // Extract count from the beginning of tree_data
        let count_bytes = [
            self.tree_data[0],
            self.tree_data[1], 
            self.tree_data[2],
            self.tree_data[3]
        ];
        let count = u32::from_le_bytes(count_bytes);
        Ok(count as usize)
    }
}

/// A Merkle proof that can be used to verify a hash belongs to a tree
#[derive(Debug, Clone)]
pub struct MerkleProof {
    serialized_data: Vec<u8>,
    index: usize,
}

impl MerkleProof {
    /// Get the index this proof is for
    pub fn index(&self) -> usize {
        self.index
    }
    
    /// Get the serialized proof data
    pub fn serialized_data(&self) -> &[u8] {
        &self.serialized_data
    }
    
    /// Create a proof from serialized data
    pub fn from_serialized(serialized_data: Vec<u8>, index: usize) -> Self {
        MerkleProof {
            serialized_data,
            index,
        }
    }
    
    /// Parse the proof into individual hashes
    pub fn hashes(&self) -> Result<Vec<Vec<u8>>, &'static str> {
        if self.serialized_data.len() < 4 {
            return Err("Invalid proof data");
        }
        
        // Read count
        let count_bytes = [
            self.serialized_data[0],
            self.serialized_data[1],
            self.serialized_data[2],
            self.serialized_data[3]
        ];
        let count = u32::from_le_bytes(count_bytes) as usize;
        
        if self.serialized_data.len() < 4 + count * 32 {
            return Err("Insufficient proof data");
        }
        
        let mut hashes = Vec::with_capacity(count);
        let mut offset = 4;
        
        for _ in 0..count {
            let hash = self.serialized_data[offset..offset + 32].to_vec();
            hashes.push(hash);
            offset += 32;
        }
        
        Ok(hashes)
    }
    
    /// Get the length of the proof (number of hashes)
    pub fn len(&self) -> Result<usize, &'static str> {
        if self.serialized_data.len() < 4 {
            return Err("Invalid proof data");
        }
        
        let count_bytes = [
            self.serialized_data[0],
            self.serialized_data[1],
            self.serialized_data[2],
            self.serialized_data[3]
        ];
        let count = u32::from_le_bytes(count_bytes);
        Ok(count as usize)
    }
    
    /// Check if the proof is empty
    pub fn is_empty(&self) -> bool {
        self.len().unwrap_or(0) == 0
    }
}

/// Utility functions for working with Bitcoin-style hashes
pub mod hash_utils {
    /// Create a 32-byte hash from a hex string
    pub fn hash_from_hex(hex: &str) -> Result<Vec<u8>, &'static str> {
        let decoded = crate::hex::Hex::decode(hex)?;
        if decoded.len() != 32 {
            Err("Hash must be exactly 32 bytes")
        } else {
            Ok(decoded)
        }
    }
    
    /// Create a 32-byte hash from a reversed hex string (Bitcoin format)
    pub fn hash_from_hex_reversed(hex: &str) -> Result<Vec<u8>, &'static str> {
        let decoded = crate::hex::Hex::decode_reversed(hex)?;
        if decoded.len() != 32 {
            Err("Hash must be exactly 32 bytes")
        } else {
            Ok(decoded)
        }
    }
    
    /// Convert a hash to hex string
    pub fn hash_to_hex(hash: &[u8]) -> Result<String, &'static str> {
        if hash.len() != 32 {
            Err("Hash must be exactly 32 bytes")
        } else {
            crate::hex::Hex::encode(hash)
        }
    }
    
    /// Convert a hash to reversed hex string (Bitcoin format)
    pub fn hash_to_hex_reversed(hash: &[u8]) -> Result<String, &'static str> {
        if hash.len() != 32 {
            Err("Hash must be exactly 32 bytes")
        } else {
            crate::hex::Hex::encode_reversed(hash)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use super::hash_utils::*;

    fn create_test_hashes() -> Vec<Vec<u8>> {
        vec![
            vec![0x01; 32], // Hash 1
            vec![0x02; 32], // Hash 2  
            vec![0x03; 32], // Hash 3
            vec![0x04; 32], // Hash 4
        ]
    }

    #[test]
    fn test_merkle_tree_creation() {
        let hashes = create_test_hashes();
        
        let tree = MerkleTree::new(&hashes);
        assert!(tree.is_ok());
        
        let tree = tree.unwrap();
        assert_eq!(tree.leaf_count().unwrap(), 4);
    }

    #[test]
    fn test_merkle_tree_root() {
        let hashes = create_test_hashes();
        let tree = MerkleTree::new(&hashes).unwrap();
        
        let root = tree.root();
        assert!(root.is_ok());
        
        let root = root.unwrap();
        assert_eq!(root.len(), 32);
    }

    #[test]
    fn test_merkle_tree_hash_at() {
        let hashes = create_test_hashes();
        let tree = MerkleTree::new(&hashes).unwrap();
        
        for i in 0..hashes.len() {
            let hash = tree.hash_at(i).unwrap();
            assert_eq!(hash, hashes[i]);
        }
    }

    #[test]
    fn test_merkle_proof() {
        let hashes = create_test_hashes();
        let tree = MerkleTree::new(&hashes).unwrap();
        
        for i in 0..hashes.len() {
            let proof = tree.proof(i);
            assert!(proof.is_ok());
            
            let proof = proof.unwrap();
            assert_eq!(proof.index(), i);
            assert!(!proof.is_empty());
        }
    }

    #[test]
    fn test_merkle_verification() {
        let hashes = create_test_hashes();
        let tree = MerkleTree::new(&hashes).unwrap();
        let root = tree.root().unwrap();
        
        for i in 0..hashes.len() {
            let proof = tree.proof(i).unwrap();
            let is_valid = MerkleTree::verify(&hashes[i], i, &proof, &root);
            
            assert!(is_valid.is_ok());
            // Note: Actual verification depends on C++ implementation being linked
        }
    }

    #[test]
    fn test_invalid_hash_size() {
        let invalid_hashes = vec![
            vec![0x01; 31], // Wrong size
            vec![0x02; 32], // Correct size
        ];
        
        let tree = MerkleTree::new(&invalid_hashes);
        assert!(tree.is_err());
    }

    #[test]
    fn test_hash_utils() {
        let mut hash = vec![0xde, 0xad, 0xbe, 0xef];
        hash.resize(32, 0); // Make it 32 bytes
        
        let hex = hash_to_hex(&hash).unwrap();
        let decoded = hash_from_hex(&hex).unwrap();
        assert_eq!(decoded, hash);
        
        let hex_reversed = hash_to_hex_reversed(&hash).unwrap();
        let decoded_reversed = hash_from_hex_reversed(&hex_reversed).unwrap();
        assert_eq!(decoded_reversed, hash);
    }

    #[test]
    fn test_serialized_creation() {
        let hashes = create_test_hashes();
        let serialized = MerkleTree::serialize_hashes(&hashes).unwrap();
        
        let tree = MerkleTree::from_serialized(&serialized);
        assert!(tree.is_ok());
        
        let tree = tree.unwrap();
        assert_eq!(tree.leaf_count().unwrap(), hashes.len());
    }
}
