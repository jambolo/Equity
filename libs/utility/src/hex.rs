//! Hexadecimal encoding and decoding utilities

use crate::ffi;

/// Hexadecimal conversion utilities
pub struct Hex;

impl Hex {
    /// Convert bytes to hexadecimal string
    pub fn encode(data: &[u8]) -> Result<String, &'static str> {
        let data_vec = data.to_vec();
        let mut output = String::new();
        
        if ffi::utilityToHex(&data_vec, &mut output) {
            Ok(output)
        } else {
            Err("Failed to encode to hex")
        }
    }
    
    /// Convert hexadecimal string to bytes
    pub fn decode(hex: &str) -> Result<Vec<u8>, &'static str> {
        let hex_string = hex.to_string();
        let mut output = Vec::new();
        
        if ffi::utilityFromHex(&hex_string, &mut output) {
            Ok(output)
        } else {
            Err("Failed to decode from hex")
        }
    }
    
    /// Convert bytes to hexadecimal string with reversed byte order
    /// This is typically used for Bitcoin hash display
    pub fn encode_reversed(data: &[u8]) -> Result<String, &'static str> {
        let data_vec = data.to_vec();
        let mut output = String::new();
        
        if ffi::utilityToHexR(&data_vec, &mut output) {
            Ok(output)
        } else {
            Err("Failed to encode to reversed hex")
        }
    }
    
    /// Convert hexadecimal string to bytes with reversed byte order
    /// This is typically used for Bitcoin hash input
    pub fn decode_reversed(hex: &str) -> Result<Vec<u8>, &'static str> {
        let hex_string = hex.to_string();
        let mut output = Vec::new();
        
        if ffi::utilityFromHexR(&hex_string, &mut output) {
            Ok(output)
        } else {
            Err("Failed to decode from reversed hex")
        }
    }
    
    /// Convert raw pointer to hex string (unsafe)
    /// 
    /// # Safety
    /// The caller must ensure that `data` points to valid memory of at least `length` bytes
    pub unsafe fn encode_ptr(data: *const u8, length: usize) -> Result<String, &'static str> {
        let mut output = String::new();
        
        if ffi::utilityToHexPtr(data, length, &mut output) {
            Ok(output)
        } else {
            Err("Failed to encode pointer to hex")
        }
    }
    
    /// Convert raw hex string pointer to bytes (unsafe)
    /// 
    /// # Safety
    /// The caller must ensure that `hex` points to valid memory of at least `length` bytes
    pub unsafe fn decode_ptr(hex: *const i8, length: usize) -> Result<Vec<u8>, &'static str> {
        let mut output = Vec::new();
        
        if ffi::utilityFromHexPtr(hex, length, &mut output) {
            Ok(output)
        } else {
            Err("Failed to decode pointer from hex")
        }
    }
    
    /// Convert raw pointer to reversed hex string (unsafe)
    /// 
    /// # Safety
    /// The caller must ensure that `data` points to valid memory of at least `length` bytes
    pub unsafe fn encode_reversed_ptr(data: *const u8, length: usize) -> Result<String, &'static str> {
        let mut output = String::new();
        
        if ffi::utilityToHexRPtr(data, length, &mut output) {
            Ok(output)
        } else {
            Err("Failed to encode pointer to reversed hex")
        }
    }
    
    /// Convert raw hex string pointer to bytes with reversed order (unsafe)
    /// 
    /// # Safety
    /// The caller must ensure that `hex` points to valid memory of at least `length` bytes
    pub unsafe fn decode_reversed_ptr(hex: *const i8, length: usize) -> Result<Vec<u8>, &'static str> {
        let mut output = Vec::new();
        
        if ffi::utilityFromHexRPtr(hex, length, &mut output) {
            Ok(output)
        } else {
            Err("Failed to decode pointer from reversed hex")
        }
    }
}

/// Trait for types that can be converted to/from hex
pub trait ToHex {
    /// Convert to hex string
    fn to_hex(&self) -> Result<String, &'static str>;
    /// Convert to hex string with reversed byte order
    fn to_hex_reversed(&self) -> Result<String, &'static str>;
}

/// Trait for types that can be created from hex
pub trait FromHex: Sized {
    /// Create from hex string
    fn from_hex(hex: &str) -> Result<Self, &'static str>;
    /// Create from hex string with reversed byte order
    fn from_hex_reversed(hex: &str) -> Result<Self, &'static str>;
}

impl ToHex for Vec<u8> {
    fn to_hex(&self) -> Result<String, &'static str> {
        Hex::encode(self)
    }
    
    fn to_hex_reversed(&self) -> Result<String, &'static str> {
        Hex::encode_reversed(self)
    }
}

impl ToHex for [u8] {
    fn to_hex(&self) -> Result<String, &'static str> {
        Hex::encode(self)
    }
    
    fn to_hex_reversed(&self) -> Result<String, &'static str> {
        Hex::encode_reversed(self)
    }
}

impl<const N: usize> ToHex for [u8; N] {
    fn to_hex(&self) -> Result<String, &'static str> {
        Hex::encode(self)
    }
    
    fn to_hex_reversed(&self) -> Result<String, &'static str> {
        Hex::encode_reversed(self)
    }
}

impl FromHex for Vec<u8> {
    fn from_hex(hex: &str) -> Result<Self, &'static str> {
        Hex::decode(hex)
    }
    
    fn from_hex_reversed(hex: &str) -> Result<Self, &'static str> {
        Hex::decode_reversed(hex)
    }
}

/// Shorten a string by replacing the middle with "..."
pub fn shorten_string(input: &str, max_size: usize) -> Result<String, &'static str> {
    let input_string = input.to_string();
    let mut output = String::new();
    
    if ffi::utilityShorten(&input_string, max_size, &mut output) {
        Ok(output)
    } else {
        Err("Failed to shorten string")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_hex_encode_decode() {
        let data = vec![0x12, 0x34, 0xab, 0xcd];
        
        let hex = Hex::encode(&data).unwrap();
        assert_eq!(hex, "1234abcd");
        
        let decoded = Hex::decode(&hex).unwrap();
        assert_eq!(decoded, data);
    }

    #[test]
    fn test_hex_encode_decode_reversed() {
        let data = vec![0x12, 0x34, 0xab, 0xcd];
        
        let hex = Hex::encode_reversed(&data).unwrap();
        assert_eq!(hex, "cdab3412");
        
        let decoded = Hex::decode_reversed(&hex).unwrap();
        assert_eq!(decoded, data);
    }

    #[test]
    fn test_trait_implementations() {
        let data = vec![0xde, 0xad, 0xbe, 0xef];
        
        let hex = data.to_hex().unwrap();
        assert_eq!(hex, "deadbeef");
        
        let decoded = Vec::<u8>::from_hex(&hex).unwrap();
        assert_eq!(decoded, data);
    }

    #[test]
    fn test_array_conversion() {
        let data = [0x01, 0x23, 0x45, 0x67];
        
        let hex = data.to_hex().unwrap();
        assert_eq!(hex, "01234567");
        
        let reversed_hex = data.to_hex_reversed().unwrap();
        assert_eq!(reversed_hex, "67452301");
    }

    #[test]
    fn test_shorten_string() {
        let long_string = "This is a very long string that needs to be shortened";
        
        let shortened = shorten_string(long_string, 20).unwrap();
        assert!(shortened.len() <= 20);
        assert!(shortened.contains("..."));
    }

    #[test]
    fn test_empty_data() {
        let empty_data = vec![];
        let hex = Hex::encode(&empty_data).unwrap();
        assert_eq!(hex, "");
        
        let decoded = Hex::decode("").unwrap();
        assert_eq!(decoded, empty_data);
    }
}
