//! Merkle Tree functionality for Bitcoin
//! 
//! Provides functionality for creating and working with Bitcoin Merkle trees.

use crate::ffi;
use crate::{Result, EquityError};

/// Bitcoin Merkle tree utilities
pub struct MerkleTree;

impl MerkleTree {
    /// Create a Merkle tree from a list of hashes
    pub fn create(hashes: &[Vec<u8>]) -> Vec<u8> {
        // Convert Vec<Vec<u8>> to a flat byte array for FFI
        let mut flat_hashes = Vec::new();
        for hash in hashes {
            flat_hashes.extend_from_slice(hash);
        }
        ffi::merkleTreeCreate(&flat_hashes, hashes.len())
    }

    /// Get the root hash of a Merkle tree
    pub fn get_root(tree_data: &[u8]) -> Vec<u8> {
        ffi::merkleTreeGetRoot(tree_data)
    }

    /// Get a Merkle proof for a specific transaction index
    pub fn get_proof(tree_data: &[u8], index: usize) -> Vec<Vec<u8>> {
        let proof_cpp = ffi::merkleTreeGetProof(tree_data, index);
        // Convert MerkleProofCpp to Vec<Vec<u8>>
        // Assuming each hash is 32 bytes (SHA256) and proof_hashes contains concatenated hashes
        let hash_size = 32;
        let mut result = Vec::new();
        for i in 0..proof_cpp.proof_count {
            let start = i * hash_size;
            let end = start + hash_size;
            if end <= proof_cpp.proof_hashes.len() {
                result.push(proof_cpp.proof_hashes[start..end].to_vec());
            }
        }
        result
    }

    /// Verify a Merkle proof
    pub fn verify(hash: &[u8], index: usize, proof: &[Vec<u8>], root: &[u8]) -> bool {
        // Convert Vec<Vec<u8>> to flat byte array for FFI
        let mut flat_proof = Vec::new();
        for proof_hash in proof {
            flat_proof.extend_from_slice(proof_hash);
        }
        ffi::merkleTreeVerify(hash, index, &flat_proof, proof.len(), root)
    }

    /// Create a Merkle tree from transaction hashes and return the root
    pub fn compute_root(hashes: &[Vec<u8>]) -> Vec<u8> {
        let tree_data = Self::create(hashes);
        Self::get_root(&tree_data)
    }

    /// Verify that a transaction is included in a block with the given Merkle root
    pub fn verify_inclusion(
        tx_hash: &[u8], 
        tx_index: usize, 
        merkle_proof: &[Vec<u8>], 
        merkle_root: &[u8]
    ) -> bool {
        Self::verify(tx_hash, tx_index, merkle_proof, merkle_root)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_single_transaction_merkle_tree() {
        let tx_hash = vec![1u8; 32];
        let hashes = vec![tx_hash.clone()];
        
        let tree_data = MerkleTree::create(&hashes);
        assert!(!tree_data.is_empty());
        
        let root = MerkleTree::get_root(&tree_data);
        assert_eq!(root.len(), 32);
        
        // For a single transaction, root should equal the transaction hash
        assert_eq!(root, tx_hash);
    }

    #[test]
    fn test_multiple_transaction_merkle_tree() {
        let hashes = vec![
            vec![1u8; 32],
            vec![2u8; 32],
            vec![3u8; 32],
            vec![4u8; 32],
        ];
        
        let tree_data = MerkleTree::create(&hashes);
        assert!(!tree_data.is_empty());
        
        let root = MerkleTree::get_root(&tree_data);
        assert_eq!(root.len(), 32);
        
        // Test proof generation and verification
        let proof = MerkleTree::get_proof(&tree_data, 0);
        let is_valid = MerkleTree::verify(&hashes[0], 0, &proof, &root);
        assert!(is_valid);
    }

    #[test]
    fn test_merkle_proof_verification() {
        let hashes = vec![
            vec![10u8; 32],
            vec![20u8; 32],
        ];
        
        let root = MerkleTree::compute_root(&hashes);
        
        // Create tree for proof generation
        let tree_data = MerkleTree::create(&hashes);
        let proof = MerkleTree::get_proof(&tree_data, 0);
        
        // Verify the first transaction
        let is_valid = MerkleTree::verify_inclusion(&hashes[0], 0, &proof, &root);
        assert!(is_valid);
        
        // Verify the second transaction
        let proof1 = MerkleTree::get_proof(&tree_data, 1);
        let is_valid1 = MerkleTree::verify_inclusion(&hashes[1], 1, &proof1, &root);
        assert!(is_valid1);
    }
}
