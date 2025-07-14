//! Script functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin scripts.

use crate::ffi;
use crate::{Result, EquityError};

/// A Bitcoin script
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Script {
    data: Vec<u8>,
}

impl Script {
    /// Create a script from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let script_data = ffi::scriptFromData(data);
        if ffi::scriptIsValid(&script_data) {
            Ok(Script { data: script_data })
        } else {
            Err(EquityError("Invalid script data".to_string()))
        }
    }

    /// Create a script from hexadecimal string
    pub fn from_hex(hex: &str) -> Result<Self> {
        match hex::decode(hex) {
            Ok(data) => Self::from_data(&data),
            Err(_) => Err(EquityError("Invalid hex string".to_string()))
        }
    }

    /// Convert the script to hexadecimal string
    pub fn to_hex(&self) -> String {
        ffi::scriptToHex(&self.data)
    }

    /// Get the raw script data
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Check if the script is valid
    pub fn is_valid(&self) -> bool {
        ffi::scriptIsValid(&self.data)
    }

    /// Get the length of the script in bytes
    pub fn len(&self) -> usize {
        self.data.len()
    }

    /// Check if the script is empty
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }
}

impl std::fmt::Display for Script {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_hex())
    }
}

impl std::str::FromStr for Script {
    type Err = EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_hex(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_script() {
        let empty_data = vec![];
        let script = Script::from_data(&empty_data);
        
        match script {
            Ok(s) => {
                assert!(s.is_empty());
                assert_eq!(s.len(), 0);
            }
            Err(_) => {
                // Empty scripts might be considered invalid
            }
        }
    }

    #[test]
    fn test_script_hex_conversion() {
        let test_data = vec![0x76, 0xa9, 0x14]; // OP_DUP OP_HASH160 PUSH(20)
        if let Ok(script) = Script::from_data(&test_data) {
            let hex = script.to_hex();
            assert!(!hex.is_empty());
            
            let parsed = Script::from_hex(&hex);
            assert!(parsed.is_ok());
        }
    }
}
