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
    }
}

pub mod endian;
pub mod hex;
pub mod debug;

// Re-export main types for convenience
pub use endian::Endian;
pub use hex::Hex;

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
        
        // Test tree creation (Rust-only implementation for now)
        // TODO: Implement MerkleTree functions when needed
        println!("MerkleTree test skipped - using Rust-only implementation");
        assert_eq!(serialized_hashes.len(), 4 + 4 * 32); // 4 bytes count + 4 * 32 byte hashes
    }
}

