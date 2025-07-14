//! SHA-512 hashing functions

use crate::ffi;

// Constants
pub const SHA512_HASH_SIZE: usize = 64; // 512 bits / 8 = 64 bytes
pub type Sha512Hash = [u8; SHA512_HASH_SIZE];

// Safe Rust wrapper using raw pointers
pub fn sha512(input: &[u8]) -> Sha512Hash {
    let mut output = [0u8; SHA512_HASH_SIZE];
    unsafe {
        ffi::sha512(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience function
pub fn sha512_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::sha512Vector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sha512_basic() {
        let input = b"hello world";
        let hash = sha512(input);
        
        // SHA-512 should always produce 64 bytes
        assert_eq!(hash.len(), SHA512_HASH_SIZE);
        
        // Test vector version
        let hash_vec = sha512_vec(input);
        assert_eq!(hash_vec.len(), SHA512_HASH_SIZE);
        
        // Both should produce the same result
        assert_eq!(hash.to_vec(), hash_vec);
    }

    #[test]
    fn test_empty_input() {
        let empty: &[u8] = &[];
        let hash = sha512(empty);
        assert_eq!(hash.len(), SHA512_HASH_SIZE);
        
        let hash_vec = sha512_vec(empty);
        assert_eq!(hash_vec.len(), SHA512_HASH_SIZE);
    }
}
