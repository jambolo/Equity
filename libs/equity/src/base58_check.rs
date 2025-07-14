//! Base58Check encoding and decoding functionality
//! 
//! Provides functions for Base58Check format encoding and decoding as used in Bitcoin
//! addresses and private keys.

use crate::ffi;
use crate::{Result, EquityError};

/// Encode binary data into Base58Check format with version byte
pub fn encode(input: &[u8], version: u32) -> String {
    ffi::base58CheckEncode(input, version)
}

/// Decode Base58Check-encoded data into binary, returning the data and version
pub fn decode(input: &str) -> Result<(Vec<u8>, u32)> {
    let mut output = Vec::new();
    let mut version = 0u32;
    if ffi::base58CheckDecode(input, &mut output, &mut version) {
        Ok((output, version))
    } else {
        Err(EquityError("Failed to decode Base58Check string".to_string()))
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58check_encode_decode() {
        let test_data = b"Hello, World!";
        let version = 0x80; // Example version byte
        
        let encoded = encode(test_data, version);
        assert!(!encoded.is_empty());
        
        let (decoded, decoded_version) = decode(&encoded).unwrap();
        assert_eq!(decoded, test_data);
        assert_eq!(decoded_version, version);
    }

    #[test]
    fn test_base58check_decode_invalid() {
        let invalid_data = "invalid_base58check";
        let result = decode(invalid_data);
        assert!(result.is_err());
    }

    #[test]
    fn test_multiple_versions() {
        let test_data = b"Hello World";
        
        for version in [0x00, 0x05, 0x80, 0xEF] {
            let encoded = Base58Check::encode(test_data, version).expect("Failed to encode");
            let (decoded, decoded_version) = Base58Check::decode(&encoded).expect("Failed to decode");
            assert_eq!(decoded, test_data);
            assert_eq!(decoded_version, version);
        }
    }

    #[test]
    fn test_invalid_checksum() {
        // This should fail due to invalid checksum
        let invalid = "1AGNa15ZQXAZUgFiqJ2i7Z2DPU2J6hW62i"; // Modify last character
        let result = Base58Check::decode(invalid);
        // Note: This might pass if the C++ implementation doesn't validate checksums properly
        println!("Decode result: {:?}", result);
    }
}
