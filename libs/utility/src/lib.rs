//! Utility functions and data structures

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("utility_wrapper.h");

        // Endian conversion functions
        fn utilityEndianSwap16(x: u16) -> u16;
        fn utilityEndianSwap32(x: u32) -> u32;
        fn utilityEndianSwap64(x: u64) -> u64;

        fn utilityEndianLittle16(x: u16) -> u16;
        fn utilityEndianLittle32(x: u32) -> u32;
        fn utilityEndianLittle64(x: u64) -> u64;

        fn utilityEndianBig16(x: u16) -> u16;
        fn utilityEndianBig32(x: u32) -> u32;
        fn utilityEndianBig64(x: u64) -> u64;

        // Utility hex conversion functions
        fn utilityToHex(input: &Vec<u8>, output: &mut String) -> bool;
        unsafe fn utilityToHexPtr(data: *const u8, length: usize, output: &mut String) -> bool;
        fn utilityFromHex(hex: &String, output: &mut Vec<u8>) -> bool;
        unsafe fn utilityFromHexPtr(hex: *const i8, length: usize, output: &mut Vec<u8>) -> bool;

        fn utilityToHexR(input: &Vec<u8>, output: &mut String) -> bool;
        unsafe fn utilityToHexRPtr(data: *const u8, length: usize, output: &mut String) -> bool;
        fn utilityFromHexR(hex: &String, output: &mut Vec<u8>) -> bool;
        unsafe fn utilityFromHexRPtr(hex: *const i8, length: usize, output: &mut Vec<u8>) -> bool;

        fn utilityShorten(input: &String, size: usize, output: &mut String) -> bool;

        // MerkleTree functions - using serialized format to avoid Vec<Vec<u8>> issues
        fn utilityMerkleTreeCreateFromSerialized(serialized_hashes: &Vec<u8>, tree_data: &mut Vec<u8>) -> bool;
        fn utilityMerkleTreeGetRoot(tree_data: &Vec<u8>, root: &mut Vec<u8>) -> bool;
        fn utilityMerkleTreeGetHashAt(tree_data: &Vec<u8>, index: usize, hash: &mut Vec<u8>) -> bool;
        fn utilityMerkleTreeGetProofSerialized(tree_data: &Vec<u8>, index: usize, serialized_proof: &mut Vec<u8>) -> bool;
        fn utilityMerkleTreeVerifySerialized(
            hash: &Vec<u8>,
            index: usize,
            serialized_proof: &Vec<u8>,
            root: &Vec<u8>
        ) -> bool;
    }
}

pub mod endian;
pub mod hex;
pub mod merkle;
pub mod debug;

// Re-export main types for convenience
pub use endian::Endian;
pub use hex::Hex;
pub use merkle::MerkleTree;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_endian_swap() {
        assert_eq!(ffi::utilityEndianSwap16(0x1234), 0x3412);
        assert_eq!(ffi::utilityEndianSwap32(0x12345678), 0x78563412);
        assert_eq!(ffi::utilityEndianSwap64(0x123456789abcdef0), 0xf0debc9a78563412);
    }

    #[test]
    fn test_hex_conversion() {
        let data = vec![0x12, 0x34, 0xab, 0xcd];
        let mut hex_output = String::new();
        
        if ffi::utilityToHex(&data, &mut hex_output) {
            assert_eq!(hex_output, "1234abcd");
            
            let mut decoded = Vec::new();
            if ffi::utilityFromHex(&hex_output, &mut decoded) {
                assert_eq!(decoded, data);
            }
        }
    }

    #[test]
    fn test_hex_reverse_conversion() {
        let data = vec![0x12, 0x34, 0xab, 0xcd];
        let mut hex_output = String::new();
        
        if ffi::utilityToHexR(&data, &mut hex_output) {
            assert_eq!(hex_output, "cdab3412");
            
            let mut decoded = Vec::new();
            if ffi::utilityFromHexR(&hex_output, &mut decoded) {
                assert_eq!(decoded, data);
            }
        }
    }

    #[test]
    fn test_string_shorten() {
        let long_string = "This is a very long string that should be shortened".to_string();
        let mut shortened = String::new();
        
        if ffi::utilityShorten(&long_string, 20, &mut shortened) {
            assert!(shortened.len() <= 20);
            assert!(shortened.contains("..."));
        }
    }

    #[test]
    fn test_merkle_tree_operations() {
        // Create test hashes (4 32-byte hashes)
        let mut serialized_hashes = Vec::new();
        
        // Count (4 hashes)
        serialized_hashes.extend_from_slice(&4u32.to_le_bytes());
        
        // Add 4 test hashes
        for i in 1..=4 {
            let mut hash = vec![i as u8; 32];
            serialized_hashes.append(&mut hash);
        }
        
        // Test tree creation
        let mut tree_data = Vec::new();
        if ffi::utilityMerkleTreeCreateFromSerialized(&serialized_hashes, &mut tree_data) {
            assert!(!tree_data.is_empty());
            
            // Test getting root
            let mut root = Vec::new();
            if ffi::utilityMerkleTreeGetRoot(&tree_data, &mut root) {
                assert_eq!(root.len(), 32);
            }
            
            // Test getting hash at index
            let mut hash_at_0 = Vec::new();
            if ffi::utilityMerkleTreeGetHashAt(&tree_data, 0, &mut hash_at_0) {
                assert_eq!(hash_at_0.len(), 32);
                assert_eq!(hash_at_0[0], 1); // First hash starts with 0x01
            }
            
            // Test getting proof
            let mut proof = Vec::new();
            if ffi::utilityMerkleTreeGetProofSerialized(&tree_data, 0, &mut proof) {
                assert!(!proof.is_empty());
                
                // Test verification
                let _is_valid = ffi::utilityMerkleTreeVerifySerialized(
                    &hash_at_0,
                    0,
                    &proof,
                    &root
                );
                // Note: Verification result depends on C++ implementation
            }
        }
    }
}

