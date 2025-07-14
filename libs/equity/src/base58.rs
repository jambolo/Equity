//! Base58 encoding and decoding functionality
//! 
//! Provides functions for Base58 format encoding and decoding as used in Bitcoin.

use crate::ffi;
use crate::{Result, EquityError};

/// Encode binary data into Base58 format
pub fn encode(input: &[u8]) -> String {
    ffi::base58Encode(input)
}

/// Decode Base58-encoded data into binary
pub fn decode(input: &str) -> Result<Vec<u8>> {
    let mut output = Vec::new();
    if ffi::base58Decode(input, &mut output) {
        Ok(output)
    } else {
        Err(EquityError("Failed to decode Base58 string".to_string()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58_encode_decode() {
        let test_data = b"Hello, World!";
        let encoded = encode(test_data);
        assert!(!encoded.is_empty());
        
        let decoded = decode(&encoded).unwrap();
        assert_eq!(decoded, test_data);
    }

    #[test]
    fn test_base58_decode_invalid() {
        let invalid_base58 = "0OIl"; // Contains invalid characters
        let result = decode(invalid_base58);
        assert!(result.is_err());
    }

    #[test]
    fn test_invalid_base58() {
        let invalid = "0OIl"; // Contains invalid Base58 characters
        let result = Base58::decode(invalid);
        assert!(result.is_err());
    }
}
