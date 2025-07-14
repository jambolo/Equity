//! SHA-1 hashing functions

use crate::ffi;

// Constants
pub const SHA1_HASH_SIZE: usize = 20; // 160 bits / 8 = 20 bytes
pub type Sha1Hash = [u8; SHA1_HASH_SIZE];

// Safe Rust wrapper using raw pointers
pub fn sha1(input: &[u8]) -> Sha1Hash {
    let mut output = [0u8; SHA1_HASH_SIZE];
    unsafe {
        ffi::sha1(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience function
pub fn sha1_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::sha1Vector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sha1_basic() {
        let input = b"hello world";
        let hash = sha1(input);
        
        // SHA-1 should always produce 20 bytes
        assert_eq!(hash.len(), SHA1_HASH_SIZE);
        
        // Test vector version
        let hash_vec = sha1_vec(input);
        assert_eq!(hash_vec.len(), SHA1_HASH_SIZE);
        
        // Both should produce the same result
        assert_eq!(hash.to_vec(), hash_vec);
    }

    #[test]
    fn test_empty_input() {
        let empty: &[u8] = &[];
        let hash = sha1(empty);
        assert_eq!(hash.len(), SHA1_HASH_SIZE);
        
        let hash_vec = sha1_vec(empty);
        assert_eq!(hash_vec.len(), SHA1_HASH_SIZE);
    }
}
