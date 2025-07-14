use crate::{ffi, Result, error};

/// Represents a Bitcoin transaction ID (hash)
#[derive(Debug, Clone, PartialEq, Eq, Hash)]
pub struct Txid {
    bytes: Vec<u8>,
}

impl Txid {
    /// Create a Txid from a hex string
    pub fn from_hex(hex: &str) -> Result<Self> {
        let mut bytes = Vec::new();
        if unsafe { ffi::txidFromString(hex, &mut bytes) } {
            Ok(Txid { bytes })
        } else {
            Err(error(&format!("Invalid txid hex string: {}", hex)))
        }
    }

    /// Create a Txid from bytes
    pub fn from_bytes(data: &[u8]) -> Result<Self> {
        if data.len() != 32 {
            return Err(error("Txid must be exactly 32 bytes"));
        }

        let bytes = data.to_vec();
        Ok(Txid { bytes })
    }

    /// Get the raw bytes of the txid
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Convert to hex string representation
    pub fn to_hex(&self) -> Result<String> {
        let mut output = String::new();
        if unsafe { ffi::txidToString(&self.bytes, &mut output) } {
            Ok(output)
        } else {
            Err(error("Failed to convert txid to hex string"))
        }
    }

    /// Check if the txid is valid
    pub fn is_valid(&self) -> bool {
        unsafe { ffi::txidIsValid(&self.bytes)
    }

    /// Create a null/zero txid
    pub fn null() -> Self {
        Txid { bytes: vec![0u8; 32] }
    }

    /// Check if this is a null/zero txid
    pub fn is_null(&self) -> bool {
        self.bytes == vec![0u8; 32]
    }
}

impl std::str::FromStr for Txid {
    type Err = crate::EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_hex(s)
    }
}

impl std::fmt::Display for Txid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.to_hex() } {
            Ok(hex) => write!(f, "{}", hex),
            Err(_) => write!(f, "{}", hex::encode(&self.bytes)),
        }
    }
}

impl std::fmt::LowerHex for Txid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", hex::encode(&self.bytes))
    }
}

impl std::fmt::UpperHex for Txid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", hex::encode_upper(&self.bytes))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_null_txid() } {
        let null_txid = Txid::null();
        assert!(null_txid.is_null());
        assert_eq!(null_txid.bytes(), &[0u8; 32]);
        println!("Null txid: {}", null_txid);
    }

    #[test]
    fn test_txid_from_bytes() } {
        let test_bytes = [0xFFu8; 32];
        let txid = Txid::from_bytes(&test_bytes).expect("Failed to create txid");
        assert_eq!(txid.bytes(), &test_bytes);
        assert!(!txid.is_null());
        println!("Txid from bytes: {}", txid);
    }

    #[test]
    fn test_invalid_txid_length() } {
        let invalid_bytes = [1u8; 31]; // Wrong length
        let result = Txid::from_bytes(&invalid_bytes);
        assert!(result.is_err());
    }

    #[test]
    fn test_txid_hex_formatting() } {
        let test_bytes = [0xABu8; 32];
        let txid = Txid::from_bytes(&test_bytes).expect("Failed to create txid");
        
        println!("Txid display: {}", txid);
        println!("Txid lower hex: {:x}", txid);
        println!("Txid upper hex: {:X}", txid);
    }

    #[test]
    fn test_txid_validation() } {
        let test_bytes = [1u8; 32];
        let txid = Txid::from_bytes(&test_bytes).expect("Failed to create txid");
        
        // The validity depends on the C++ implementation
        let is_valid = txid.is_valid();
        println!("Txid is valid: {}", is_valid);
    }
}
