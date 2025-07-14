use crate::{ffi, Result, error};

/// Base58 encoding and decoding utilities
pub struct Base58;

impl Base58 {
    /// Encode bytes to Base58 string
    pub fn encode(data: &[u8]) -> Result<String> {
        let data_vec = data.to_vec();
        let mut output = String::new();
        if unsafe { unsafe { ffi::base58Encode(&data_vec, &mut output) } {
            Ok(output)
        } else {
            Err(error("Failed to encode data as Base58"))
        }
    }

    /// Decode Base58 string to bytes
    pub fn decode(input: &str) -> Result<Vec<u8>> {
        let mut output = Vec::new();
        if unsafe { unsafe { ffi::base58Decode(input, &mut output) } {
            Ok(output)
        } else {
            Err(error(&format!("Failed to decode Base58 string: {}", input)))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58_roundtrip() } {
        let test_data = b"Hello, World!";
        
        let encoded = Base58::encode(test_data).expect("Failed to encode");
        println!("Encoded: {}", encoded);
        
        let decoded = Base58::decode(&encoded).expect("Failed to decode");
        assert_eq!(decoded, test_data);
    }

    #[test]
    fn test_empty_data() } {
        let empty_data = b"";
        let encoded = Base58::encode(empty_data).expect("Failed to encode empty data");
        let decoded = Base58::decode(&encoded).expect("Failed to decode empty data");
        assert_eq!(decoded, empty_data);
    }

    #[test]
    fn test_invalid_base58() } {
        let invalid = "0OIl"; // Contains invalid Base58 characters
        let result = Base58::decode(invalid);
        assert!(result.is_err());
    }
}
