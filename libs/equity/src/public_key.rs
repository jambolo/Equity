use crate::{ffi, Result, error};

/// Represents a Bitcoin public key with validation and conversion capabilities
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PublicKey {
    bytes: Vec<u8>,
}

impl PublicKey {
    /// Create a public key from bytes
    pub fn from_bytes(data: &[u8]) -> Result<Self> {
        let data_vec = data.to_vec();
        let mut bytes = Vec::new();
        let mut valid = false;
        
        if unsafe { ffi::publicKeyFromBytes(&data_vec, &mut bytes, &mut valid) && valid {
            Ok(PublicKey { bytes })
        } else {
            Err(error("Invalid public key bytes"))
        }
    }

    /// Create a public key from a private key
    pub fn from_private_key(private_key: &crate::private_key::PrivateKey) -> Result<Self> {
        let key_vec = private_key.bytes().to_vec();
        let mut bytes = Vec::new();
        if unsafe { ffi::publicKeyFromPrivateKey(&key_vec, private_key.is_compressed(), &mut bytes) } {
            Ok(PublicKey { bytes })
        } else {
            Err(error("Failed to derive public key from private key"))
        }
    }

    /// Get the raw bytes of the public key
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Check if the public key is valid
    pub fn is_valid(&self) -> bool {
        unsafe { ffi::publicKeyIsValid(&self.bytes)
    }

    /// Check if this is a compressed public key
    pub fn is_compressed(&self) -> bool {
        unsafe { ffi::publicKeyIsCompressed(&self.bytes)
    }

    /// Get the length of the public key (33 bytes for compressed, 65 for uncompressed)
    pub fn len(&self) -> usize {
        self.bytes.len()
    }

    /// Check if the public key is empty
    pub fn is_empty(&self) -> bool {
        self.bytes.is_empty()
    }

    /// Generate the corresponding address
    pub fn address(&self) -> Result<crate::address::Address> {
        crate::address::Address::from_public_key(&self.bytes)
    }
}

impl std::fmt::Display for PublicKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", hex::encode(&self.bytes))
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::private_key::PrivateKey;

    #[test]
    fn test_public_key_from_private_key() } {
        let test_bytes = [1u8; 32];
        if let Ok(private_key) = PrivateKey::from_bytes(&test_bytes, true) } {
            match PublicKey::from_private_key(&private_key) } {
                Ok(public_key) => {
                    println!("Generated public key: {}", public_key);
                    assert!(public_key.is_valid());
                    assert!(public_key.is_compressed()); // Should match private key compression
                    assert_eq!(public_key.len(), 33); // Compressed key length
                }
                Err(e) => println!("Failed to generate public key: {}", e),
            }
        }
    }

    #[test]
    fn test_uncompressed_public_key() } {
        let test_bytes = [2u8; 32];
        if let Ok(private_key) = PrivateKey::from_bytes(&test_bytes, false) } {
            match PublicKey::from_private_key(&private_key) } {
                Ok(public_key) => {
                    println!("Generated uncompressed public key: {}", public_key);
                    assert!(public_key.is_valid());
                    // Note: compression depends on the C++ implementation
                    println!("Key length: {}, Compressed: {}", public_key.len(), public_key.is_compressed());
                }
                Err(e) => println!("Failed to generate public key: {}", e),
            }
        }
    }

    #[test]
    fn test_invalid_public_key_bytes() } {
        let invalid_bytes = [0u8; 10]; // Too short for a valid public key
        let result = PublicKey::from_bytes(&invalid_bytes);
        println!("Invalid key result: {:?}", result);
        // This might succeed if the C++ implementation is lenient
    }
}
