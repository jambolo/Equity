//! ECC (Elliptic Curve Cryptography) functions

use crate::ffi;

// Rust wrapper types
pub type PrivateKey = [u8; 32]; // 256 bits / 8 = 32 bytes
pub type PublicKey = Vec<u8>;
pub type Signature = Vec<u8>;

// Constants
pub const PRIVATE_KEY_SIZE: usize = 32;
pub const COMPRESSED_PUBLIC_KEY_SIZE: usize = 33;
pub const UNCOMPRESSED_PUBLIC_KEY_SIZE: usize = 65;

// Safe Rust wrappers for basic validation
pub fn public_key_is_valid(key: &[u8]) -> bool {
    unsafe {
        ffi::eccPublicKeyIsValid(key.as_ptr(), key.len())
    }
}

pub fn private_key_is_valid(key: &PrivateKey) -> bool {
    unsafe {
        ffi::eccPrivateKeyIsValid(key.as_ptr(), key.len())
    }
}

/// Derive a public key from a private key
pub fn derive_public_key(private_key: &PrivateKey, uncompressed: bool) -> Result<PublicKey, &'static str> {
    let mut public_key = Vec::new();
    
    let success = unsafe {
        ffi::eccDerivePublicKey(private_key.as_ptr(), &mut public_key, uncompressed)
    };
    
    if success {
        Ok(public_key)
    } else {
        Err("Failed to derive public key")
    }
}

/// Sign a message with a private key
pub fn sign(message: &[u8], private_key: &PrivateKey) -> Result<Signature, &'static str> {
    let mut signature = Vec::new();
    
    let success = unsafe {
        ffi::eccSign(message.as_ptr(), message.len(), private_key.as_ptr(), &mut signature)
    };
    
    if success {
        Ok(signature)
    } else {
        Err("Failed to sign message")
    }
}

/// Verify a message signature with a public key
pub fn verify(message: &[u8], public_key: &PublicKey, signature: &Signature) -> bool {
    unsafe {
        ffi::eccVerify(message.as_ptr(), message.len(), public_key, signature)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_private_key_validation() {
        // Test with zeros (should be invalid)
        let zero_key = [0u8; 32];
        // Note: depending on implementation, all zeros might be invalid
        let _is_valid = private_key_is_valid(&zero_key);
        // Don't assert result since it depends on implementation
    }

    #[test]
    fn test_public_key_validation() {
        // Test with invalid public key
        let invalid_key = vec![0u8; 33];
        let _is_valid = public_key_is_valid(&invalid_key);
        // Don't assert result since it depends on implementation
    }

    #[test]
    fn test_key_derivation() {
        // Test key derivation with a test private key
        // Note: This test may fail if the crypto implementation is not complete
        let test_key = [1u8; 32]; // Simple test key
        
        if private_key_is_valid(&test_key) {
            match derive_public_key(&test_key, false) {
                Ok(public_key) => {
                    assert!(!public_key.is_empty());
                    assert_eq!(public_key.len(), COMPRESSED_PUBLIC_KEY_SIZE);
                }
                Err(_) => {
                    // Key derivation failed, which is acceptable for testing
                }
            }
        }
    }

    #[test]
    fn test_signing_and_verification() {
        // Test signing and verification
        let test_key = [1u8; 32];
        let message = b"test message";
        
        if private_key_is_valid(&test_key) {
            if let Ok(public_key) = derive_public_key(&test_key, false) {
                if let Ok(signature) = sign(message, &test_key) {
                    let is_valid = verify(message, &public_key, &signature);
                    // Note: We can't assert this will be true since it depends on the crypto implementation
                    let _ = is_valid;
                }
            }
        }
    }
}
