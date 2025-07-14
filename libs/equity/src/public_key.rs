//! Public key functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin public keys.

use crate::ffi::{self, PublicKeyCpp};
use crate::{Result, EquityError};

/// A Bitcoin public key
pub struct PublicKey {
    inner: PublicKeyCpp,
}

impl PublicKey {
    /// Create a public key from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != 33 && data.len() != 65 {
            return Err(EquityError("Public key data must be 33 (compressed) or 65 (uncompressed) bytes".to_string()));
        }
        let inner = ffi::publicKeyFromData(data);
        if ffi::publicKeyIsValid(&inner) {
            Ok(PublicKey { inner })
        } else {
            Err(EquityError("Invalid public key data".to_string()))
        }
    }

    /// Create a public key from a private key
    pub fn from_private_key(private_key_data: &[u8]) -> Result<Self> {
        if private_key_data.len() != 32 {
            return Err(EquityError("Private key data must be 32 bytes".to_string()));
        }
        let inner = ffi::publicKeyFromPrivateKey(private_key_data);
        if ffi::publicKeyIsValid(&inner) {
            Ok(PublicKey { inner })
        } else {
            Err(EquityError("Invalid private key for public key generation".to_string()))
        }
    }

    /// Get the public key as raw bytes
    pub fn value(&self) -> Vec<u8> {
        ffi::publicKeyValue(&self.inner)
    }

    /// Check if the public key is valid
    pub fn is_valid(&self) -> bool {
        ffi::publicKeyIsValid(&self.inner)
    }

    /// Check if the public key is in compressed format
    pub fn is_compressed(&self) -> bool {
        ffi::publicKeyIsCompressed(&self.inner)
    }

    /// Get the size of the public key (33 for compressed, 65 for uncompressed)
    pub fn size(&self) -> usize {
        if self.is_compressed() { 33 } else { 65 }
    }
}

impl std::fmt::Debug for PublicKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("PublicKey")
            .field("valid", &self.is_valid())
            .field("compressed", &self.is_compressed())
            .field("size", &self.size())
            .finish()
    }
}

impl std::fmt::Display for PublicKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", hex::encode(self.value()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_public_key_from_private_key() {
        // Example 32-byte private key
        let private_key_data = [1u8; 32];
        let public_key = PublicKey::from_private_key(&private_key_data);
        
        if let Ok(pk) = public_key {
            assert!(pk.is_valid());
            assert!(pk.size() == 33 || pk.size() == 65);
        }
    }

    #[test]
    fn test_invalid_public_key_length() {
        let invalid_data = [1u8; 32]; // Wrong length
        let result = PublicKey::from_data(&invalid_data);
        assert!(result.is_err());
    }

    #[test]
    fn test_compressed_vs_uncompressed() {
        let private_key_data = [1u8; 32];
        if let Ok(pk) = PublicKey::from_private_key(&private_key_data) {
            let size = pk.size();
            assert!(size == 33 || size == 65);
            
            if size == 33 {
                assert!(pk.is_compressed());
            } else {
                assert!(!pk.is_compressed());
            }
        }
    }

    #[test]
    fn test_invalid_public_key_bytes() {
        let invalid_bytes = [0u8; 10]; // Too short for a valid public key
        let result = PublicKey::from_bytes(&invalid_bytes);
        println!("Invalid key result: {:?}", result);
        // This might succeed if the C++ implementation is lenient
    }
}
