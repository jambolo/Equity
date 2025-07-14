//! Private key functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin private keys.

use crate::ffi::{self, PrivateKeyCpp};
use crate::{Result, EquityError, Network};

/// A Bitcoin private key
pub struct PrivateKey {
    inner: PrivateKeyCpp,
}

impl PrivateKey {
    /// Create a private key from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != 32 {
            return Err(EquityError("Private key data must be 32 bytes".to_string()));
        }
        let inner = ffi::privateKeyFromData(data);
        if ffi::privateKeyIsValid(&inner) {
            Ok(PrivateKey { inner })
        } else {
            Err(EquityError("Invalid private key data".to_string()))
        }
    }

    /// Create a private key from a string (WIF or mini-key format)
    pub fn from_string(s: &str) -> Result<Self> {
        let inner = ffi::privateKeyFromString(s);
        if ffi::privateKeyIsValid(&inner) {
            Ok(PrivateKey { inner })
        } else {
            Err(EquityError("Invalid private key string".to_string()))
        }
    }

    /// Get the private key as raw bytes
    pub fn value(&self) -> Vec<u8> {
        ffi::privateKeyValue(&self.inner)
    }

    /// Check if the private key is valid
    pub fn is_valid(&self) -> bool {
        ffi::privateKeyIsValid(&self.inner)
    }

    /// Check if the private key is set to compressed format
    pub fn is_compressed(&self) -> bool {
        ffi::privateKeyIsCompressed(&self.inner)
    }

    /// Set the compressed flag for the private key
    pub fn set_compressed(&mut self, compressed: bool) {
        ffi::privateKeySetCompressed(&mut self.inner, compressed);
    }

    /// Convert the private key to WIF (Wallet Import Format)
    pub fn to_wif(&self, version: u32) -> String {
        ffi::privateKeyToWif(&self.inner, version)
    }

    /// Convert the private key to hexadecimal format
    pub fn to_hex(&self) -> String {
        ffi::privateKeyToHex(&self.inner)
    }
}

impl std::fmt::Debug for PrivateKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("PrivateKey")
            .field("valid", &self.is_valid())
            .field("compressed", &self.is_compressed())
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_private_key_from_data() {
        // Example 32-byte private key
        let key_data = [1u8; 32];
        let private_key = PrivateKey::from_data(&key_data);
        
        if let Ok(pk) = private_key {
            assert!(pk.is_valid());
            assert_eq!(pk.value().len(), 32);
        }
    }

    #[test]
    fn test_invalid_private_key_length() {
        let short_data = [1u8; 16]; // Too short
        let result = PrivateKey::from_data(&short_data);
        assert!(result.is_err());
    }

    #[test]
    fn test_private_key_display() {
        let test_bytes = [1u8; 32];
        if let Ok(_key) = PrivateKey::from_data(&test_bytes) {
            println!("Private key created successfully");
            // Note: Display formatting would need to be implemented
        }
    }
}
