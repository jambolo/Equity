//! SHA-256 hashing functions

use crate::ffi;

// Constants
pub const SHA256_HASH_SIZE: usize = 32; // 256 bits / 8 = 32 bytes
pub const CHECKSUM_SIZE: usize = 4; // 4 bytes
pub type Sha256Hash = [u8; SHA256_HASH_SIZE];
pub type Checksum = [u8; CHECKSUM_SIZE];

// Safe Rust wrappers using raw pointers (original functions)
pub fn sha256(input: &[u8]) -> Sha256Hash {
    let mut output = [0u8; SHA256_HASH_SIZE];
    unsafe {
        ffi::sha256(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

pub fn double_sha256(input: &[u8]) -> Sha256Hash {
    let mut output = [0u8; SHA256_HASH_SIZE];
    unsafe {
        ffi::doubleSha256(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

pub fn checksum(input: &[u8]) -> Checksum {
    let mut output = [0u8; CHECKSUM_SIZE];
    unsafe {
        ffi::checksum(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience functions
pub fn sha256_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::sha256Vector(&input_vec, &mut output);
    output
}

pub fn double_sha256_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::doubleSha256Vector(&input_vec, &mut output);
    output
}

pub fn checksum_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::checksumVector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sha256_basic() {
        let input = b"hello world";
        let hash = sha256(input);
        
        // SHA-256 should always produce 32 bytes
        assert_eq!(hash.len(), SHA256_HASH_SIZE);
        
        // Test vector version
        let hash_vec = sha256_vec(input);
        assert_eq!(hash_vec.len(), SHA256_HASH_SIZE);
        
        // Both should produce the same result
        assert_eq!(hash.to_vec(), hash_vec);
    }

    #[test]
    fn test_double_sha256() {
        let input = b"test";
        let hash = double_sha256(input);
        let hash_vec = double_sha256_vec(input);
        
        assert_eq!(hash.len(), SHA256_HASH_SIZE);
        assert_eq!(hash_vec.len(), SHA256_HASH_SIZE);
        assert_eq!(hash.to_vec(), hash_vec);
    }

    #[test]
    fn test_checksum() {
        let input = b"test";
        let checksum_result = checksum(input);
        let checksum_vec_result = checksum_vec(input);
        
        assert_eq!(checksum_result.len(), CHECKSUM_SIZE);
        assert_eq!(checksum_vec_result.len(), CHECKSUM_SIZE);
        assert_eq!(checksum_result.to_vec(), checksum_vec_result);
    }

    #[test]
    fn test_empty_input() {
        let empty: &[u8] = &[];
        let hash = sha256(empty);
        assert_eq!(hash.len(), SHA256_HASH_SIZE);
        
        let hash_vec = sha256_vec(empty);
        assert_eq!(hash_vec.len(), SHA256_HASH_SIZE);
    }
}
