//! RIPEMD-160 hashing functions

use crate::ffi;

// Constants
pub const RIPEMD160_HASH_SIZE: usize = 20; // 160 bits / 8 = 20 bytes
pub type Ripemd160Hash = [u8; RIPEMD160_HASH_SIZE];

// Safe Rust wrapper using raw pointers
pub fn ripemd160(input: &[u8]) -> Ripemd160Hash {
    let mut output = [0u8; RIPEMD160_HASH_SIZE];
    unsafe {
        ffi::ripemd160(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience function
pub fn ripemd160_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::ripemd160Vector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_ripemd160_basic() {
        let input = b"hello world";
        let hash = ripemd160(input);
        
        // RIPEMD-160 should always produce 20 bytes
        assert_eq!(hash.len(), RIPEMD160_HASH_SIZE);
        
        // Test vector version
        let hash_vec = ripemd160_vec(input);
        assert_eq!(hash_vec.len(), RIPEMD160_HASH_SIZE);
        
        // Both should produce the same result
        assert_eq!(hash.to_vec(), hash_vec);
    }

    #[test]
    fn test_empty_input() {
        let empty: &[u8] = &[];
        let hash = ripemd160(empty);
        assert_eq!(hash.len(), RIPEMD160_HASH_SIZE);
        
        let hash_vec = ripemd160_vec(empty);
        assert_eq!(hash_vec.len(), RIPEMD160_HASH_SIZE);
    }
}
