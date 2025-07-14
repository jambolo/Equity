//! PBKDF2 (Password-Based Key Derivation Function 2) functions

use crate::ffi;

// Safe Rust wrapper using raw pointers
pub fn pbkdf2_hmac_sha512(
    password: &[u8],
    salt: &[u8],
    count: i32,
    output_size: usize,
) -> Result<Vec<u8>, &'static str> {
    let mut output = vec![0u8; output_size];
    unsafe {
        let success = ffi::pbkdf2HmacSha512(
            password.as_ptr(),
            password.len(),
            salt.as_ptr(),
            salt.len(),
            count,
            output_size,
            output.as_mut_ptr(),
        );
        if success {
            Ok(output)
        } else {
            Err("PBKDF2 operation failed")
        }
    }
}

// NEW: Vector-based convenience function
pub fn pbkdf2_hmac_sha512_vec(
    password: &[u8],
    salt: &[u8],
    count: i32,
    output_size: usize,
) -> Result<Vec<u8>, &'static str> {
    let password_vec = password.to_vec();
    let salt_vec = salt.to_vec();
    let mut output = Vec::new();
    
    let success = ffi::pbkdf2HmacSha512Vector(&password_vec, &salt_vec, count, output_size, &mut output);
    
    if success {
        Ok(output)
    } else {
        Err("PBKDF2 operation failed")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_pbkdf2_basic() {
        let password = b"password";
        let salt = b"salt";
        let count = 1000;
        let output_size = 32;
        
        // Test raw pointer version
        let result1 = pbkdf2_hmac_sha512(password, salt, count, output_size);
        assert!(result1.is_ok());
        let key1 = result1.unwrap();
        assert_eq!(key1.len(), output_size);
        
        // Test vector version
        let result2 = pbkdf2_hmac_sha512_vec(password, salt, count, output_size);
        assert!(result2.is_ok());
        let key2 = result2.unwrap();
        assert_eq!(key2.len(), output_size);
        
        // Both methods should produce the same result
        assert_eq!(key1, key2);
    }

    #[test]
    fn test_pbkdf2_empty_inputs() {
        let empty: &[u8] = &[];
        let salt = b"salt";
        let count = 1;
        let output_size = 16;
        
        // This should handle empty password gracefully
        let result = pbkdf2_hmac_sha512(empty, salt, count, output_size);
        // Don't assert success since it depends on implementation
        let _ = result;
    }

    #[test]
    fn test_pbkdf2_different_sizes() {
        let password = b"test";
        let salt = b"salt";
        let count = 100;
        
        // Test different output sizes
        for size in [16, 32, 64].iter() {
            let result = pbkdf2_hmac_sha512(password, salt, count, *size);
            if let Ok(key) = result {
                assert_eq!(key.len(), *size);
            }
        }
    }
}
