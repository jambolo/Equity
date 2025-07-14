//! HMAC (Hash-based Message Authentication Code) functions

use crate::ffi;

// Constants
pub const HMAC_SHA512_SIZE: usize = 64; // 512 bits / 8 = 64 bytes
pub type HmacSha512 = [u8; HMAC_SHA512_SIZE];

// Safe Rust wrapper using raw pointers
pub fn hmac_sha512(key: &[u8], message: &[u8]) -> HmacSha512 {
    let mut output = [0u8; HMAC_SHA512_SIZE];
    unsafe {
        ffi::hmacSha512(key.as_ptr(), key.len(), message.as_ptr(), message.len(), output.as_mut_ptr());
    }
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hmac_sha512_basic() {
        let key = b"secret_key";
        let message = b"hello world";
        let hmac = hmac_sha512(key, message);
        
        // HMAC-SHA512 should always produce 64 bytes
        assert_eq!(hmac.len(), HMAC_SHA512_SIZE);
    }

    #[test]
    fn test_hmac_sha512_empty_inputs() {
        let empty: &[u8] = &[];
        let message = b"message";
        let key = b"key";
        
        // Test with empty key
        let hmac1 = hmac_sha512(empty, message);
        assert_eq!(hmac1.len(), HMAC_SHA512_SIZE);
        
        // Test with empty message
        let hmac2 = hmac_sha512(key, empty);
        assert_eq!(hmac2.len(), HMAC_SHA512_SIZE);
        
        // Test with both empty
        let hmac3 = hmac_sha512(empty, empty);
        assert_eq!(hmac3.len(), HMAC_SHA512_SIZE);
    }

    #[test]
    fn test_hmac_sha512_deterministic() {
        let key = b"test_key";
        let message = b"test_message";
        
        // Same inputs should produce same output
        let hmac1 = hmac_sha512(key, message);
        let hmac2 = hmac_sha512(key, message);
        assert_eq!(hmac1, hmac2);
        
        // Different key should produce different output
        let different_key = b"different_key";
        let hmac3 = hmac_sha512(different_key, message);
        assert_ne!(hmac1, hmac3);
        
        // Different message should produce different output
        let different_message = b"different_message";
        let hmac4 = hmac_sha512(key, different_message);
        assert_ne!(hmac1, hmac4);
    }
}
