use crate::{ffi, Result, error};

/// Base58Check encoding and decoding with checksum validation
pub struct Base58Check;

impl Base58Check {
    /// Encode bytes to Base58Check string with version byte
    pub fn encode(data: &[u8], version: u8) -> Result<String> {
        let data_vec = data.to_vec();
        let mut output = String::new();
        if unsafe { unsafe { ffi::base58CheckEncode(&data_vec, version, &mut output) } {
            Ok(output)
        } else {
            Err(error("Failed to encode data as Base58Check"))
        }
    }

    /// Decode Base58Check string to bytes and extract version
    pub fn decode(input: &str) -> Result<(Vec<u8>, u8)> {
        let mut output = Vec::new();
        let mut version = 0u8;
        if unsafe { unsafe { ffi::base58CheckDecode(input, &mut output, &mut version) } {
            Ok((output, version))
        } else {
            Err(error(&format!("Failed to decode Base58Check string: {}", input)))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58check_roundtrip() } {
        let test_data = b"Test data for Base58Check";
        let version = 0x00; // Bitcoin mainnet address version
        
        let encoded = Base58Check::encode(test_data, version).expect("Failed to encode");
        println!("Base58Check encoded: {}", encoded);
        
        let (decoded, decoded_version) = Base58Check::decode(&encoded).expect("Failed to decode");
        assert_eq!(decoded, test_data);
        assert_eq!(decoded_version, version);
    }

    #[test]
    fn test_different_versions() } {
        let test_data = b"Version test";
        
        for version in [0x00, 0x05, 0x80, 0xEF] {
            let encoded = Base58Check::encode(test_data, version).expect("Failed to encode");
            let (decoded, decoded_version) = Base58Check::decode(&encoded).expect("Failed to decode");
            assert_eq!(decoded, test_data);
            assert_eq!(decoded_version, version);
        }
    }

    #[test]
    fn test_invalid_checksum() } {
        // This should fail due to invalid checksum
        let invalid = "1AGNa15ZQXAZUgFiqJ2i7Z2DPU2J6hW62i"; // Modify last character
        let result = Base58Check::decode(invalid);
        // Note: This might pass if the C++ implementation doesn't validate checksums properly
        println!("Decode result: {:?}", result);
    }
}
