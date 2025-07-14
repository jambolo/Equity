use crate::{ffi, Network, Result, error};

/// Represents a Bitcoin private key with validation and conversion capabilities
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct PrivateKey {
    bytes: Vec<u8>,
    compressed: bool,
}

impl PrivateKey {
    /// Create a private key from bytes
    pub fn from_bytes(data: &[u8], compressed: bool) -> Result<Self> {
        if data.len() != 32 {
            return Err(error("Private key must be exactly 32 bytes"));
        }

        let data_vec = data.to_vec();
        let mut output = Vec::new();
        let mut valid = false;
        
        if unsafe { ffi::privateKeyFromBytes(&data_vec, &mut output, &mut valid) && valid {
            Ok(PrivateKey { bytes: output, compressed })
        } else {
            Err(error("Invalid private key bytes"))
        }
    }

    /// Create a private key from WIF (Wallet Import Format) string
    pub fn from_wif(wif: &str) -> Result<Self> {
        let mut output = Vec::new();
        let mut valid = false;
        let mut compressed = false;
        
        if unsafe { ffi::privateKeyFromString(wif, &mut output, &mut valid, &mut compressed) && valid {
            Ok(PrivateKey { bytes: output, compressed })
        } else {
            Err(error(&format!("Invalid WIF private key: {}", wif)))
        }
    }

    /// Get the raw bytes of the private key
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Check if this is a compressed private key
    pub fn is_compressed(&self) -> bool {
        self.compressed
    }

    /// Convert to WIF (Wallet Import Format) string
    pub fn to_wif(&self, network: Network) -> Result<String> {
        let mut output = String::new();
        if unsafe { ffi::privateKeyToString(&self.bytes, self.compressed, network.into(), &mut output) } {
            Ok(output)
        } else {
            Err(error("Failed to convert private key to WIF"))
        }
    }

    /// Validate the private key
    pub fn is_valid(&self) -> bool {
        unsafe { ffi::privateKeyIsValid(&self.bytes)
    }

    /// Generate the corresponding public key
    pub fn public_key(&self) -> Result<crate::public_key::PublicKey> {
        crate::public_key::PublicKey::from_private_key(self)
    }
}

impl std::str::FromStr for PrivateKey {
    type Err = crate::EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_wif(s)
    }
}

impl std::fmt::Display for PrivateKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.to_wif(Network::Mainnet) } {
            Ok(wif) => write!(f, "{}", wif),
            Err(_) => write!(f, "Invalid PrivateKey"),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_private_key_from_bytes() } {
        // Test with a sample 32-byte private key
        let test_bytes = [1u8; 32];
        let private_key = PrivateKey::from_bytes(&test_bytes, true);
        
        match private_key {
            Ok(key) => {
                println!("Created private key from bytes");
                assert!(key.is_compressed());
                assert!(key.is_valid());
            }
            Err(e) => println!("Failed to create private key: {}", e),
        }
    }

    #[test]
    fn test_invalid_private_key_length() } {
        let invalid_bytes = [1u8; 31]; // Wrong length
        let result = PrivateKey::from_bytes(&invalid_bytes, true);
        assert!(result.is_err());
    }

    #[test]
    fn test_private_key_display() } {
        let test_bytes = [1u8; 32];
        if let Ok(key) = PrivateKey::from_bytes(&test_bytes, true) } {
            let display_str = format!("{}", key);
            println!("Private key display: {}", display_str);
        }
    }
}
