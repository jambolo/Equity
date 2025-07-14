//! Transaction ID functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin transaction IDs.

use crate::ffi;
use crate::{Result, EquityError};

/// A Bitcoin transaction ID (TXID)
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Txid {
    hash: Vec<u8>,
}

impl Txid {
    /// Create a TXID from binary hash data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != 32 {
            return Err(EquityError("TXID hash must be 32 bytes".to_string()));
        }
        let hash = ffi::txidFromData(data);
        Ok(Txid { hash })
    }

    /// Create a TXID from JSON string
    pub fn from_json(json: &str) -> Result<Self> {
        let hash = ffi::txidFromJson(json);
        if hash.len() == 32 {
            Ok(Txid { hash })
        } else {
            Err(EquityError("Invalid TXID JSON".to_string()))
        }
    }

    /// Convert the TXID to JSON string
    pub fn to_json(&self) -> String {
        ffi::txidToJson(&self.hash)
    }

    /// Serialize the TXID to binary data
    pub fn serialize(&self) -> Vec<u8> {
        ffi::txidSerialize(&self.hash)
    }

    /// Get the raw hash bytes
    pub fn hash(&self) -> &[u8] {
        &self.hash
    }

    /// Convert to hexadecimal string (little-endian)
    pub fn to_hex(&self) -> String {
        hex::encode(&self.hash)
    }

    /// Convert to hexadecimal string (big-endian, typical display format)
    pub fn to_hex_be(&self) -> String {
        let mut reversed = self.hash.clone();
        reversed.reverse();
        hex::encode(reversed)
    }
}

impl std::fmt::Display for Txid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_hex_be())
    }
}

impl std::str::FromStr for Txid {
    type Err = EquityError;

    fn from_str(s: &str) -> Result<Self> {
        if s.len() != 64 {
            return Err(EquityError("TXID hex string must be 64 characters".to_string()));
        }
        
        match hex::decode(s) {
            Ok(mut bytes) => {
                if bytes.len() != 32 {
                    return Err(EquityError("TXID must be 32 bytes".to_string()));
                }
                // Reverse for little-endian internal representation
                bytes.reverse();
                Self::from_data(&bytes)
            }
            Err(_) => Err(EquityError("Invalid hex string".to_string()))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_txid_from_data() {
        let hash_data = [1u8; 32];
        let txid = Txid::from_data(&hash_data).unwrap();
        assert_eq!(txid.hash().len(), 32);
        assert_eq!(txid.hash(), &hash_data);
    }

    #[test]
    fn test_txid_hex_conversion() {
        let hash_data = [1u8; 32];
        let txid = Txid::from_data(&hash_data).unwrap();
        
        let hex_le = txid.to_hex();
        let hex_be = txid.to_hex_be();
        
        assert_eq!(hex_le.len(), 64);
        assert_eq!(hex_be.len(), 64);
        assert_ne!(hex_le, hex_be); // Should be different due to endianness
    }

    #[test]
    fn test_txid_from_hex_string() {
        let hex_string = "0101010101010101010101010101010101010101010101010101010101010101";
        let txid = hex_string.parse::<Txid>().unwrap();
        assert_eq!(txid.to_hex_be(), hex_string);
    }

    #[test]
    fn test_invalid_txid_length() {
        let short_data = [1u8; 16];
        let result = Txid::from_data(&short_data);
        assert!(result.is_err());
    }

    #[test]
    fn test_txid_creation() {
        let test_bytes = [0xFFu8; 32];
        let txid = Txid::from_data(&test_bytes).expect("Failed to create txid");
        println!("Txid created: {}", txid);
    }

    #[test]
    fn test_txid_hex_formatting() {
        let test_bytes = [0xABu8; 32];
        let txid = Txid::from_data(&test_bytes).expect("Failed to create txid");
        
        println!("Txid display: {}", txid);
        // Note: Custom hex formatting would need to be implemented
    }

    #[test]
    fn test_txid_validation() {
        let test_bytes = [1u8; 32];
        let txid = Txid::from_data(&test_bytes).expect("Failed to create txid");
        
        // Test basic properties
        println!("Txid created successfully");
        // Note: Validation methods would need to be implemented
    }
}
