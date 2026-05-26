//! Hexadecimal encoding and decoding utilities.

pub struct Hex;

impl Hex {
    pub fn encode(data: &[u8]) -> Result<String, &'static str> {
        Ok(hex::encode(data))
    }

    pub fn decode(s: &str) -> Result<Vec<u8>, &'static str> {
        hex::decode(s).map_err(|_| "Failed to decode from hex")
    }

    pub fn encode_reversed(data: &[u8]) -> Result<String, &'static str> {
        let mut reversed: Vec<u8> = data.iter().rev().copied().collect();
        let out = hex::encode(&reversed);
        reversed.clear();
        Ok(out)
    }

    pub fn decode_reversed(s: &str) -> Result<Vec<u8>, &'static str> {
        let mut bytes = hex::decode(s).map_err(|_| "Failed to decode from reversed hex")?;
        bytes.reverse();
        Ok(bytes)
    }
}

pub trait ToHex {
    fn to_hex(&self) -> Result<String, &'static str>;
    fn to_hex_reversed(&self) -> Result<String, &'static str>;
}

pub trait FromHex: Sized {
    fn from_hex(hex: &str) -> Result<Self, &'static str>;
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
    fn from_hex(s: &str) -> Result<Self, &'static str> {
        Hex::decode(s)
    }

    fn from_hex_reversed(s: &str) -> Result<Self, &'static str> {
        Hex::decode_reversed(s)
    }
}

/// Shortens a string by replacing the middle with "..." so the result
/// is at most `max_size` characters long.
///
/// Mirrors the legacy `Utility::shorten` semantics: when `max_size` is
/// less than the ellipsis itself (3 chars), the input is just truncated.
pub fn shorten_string(input: &str, max_size: usize) -> Result<String, &'static str> {
    const ELLIPSIS: &str = "...";
    const ELLIPSIS_LEN: usize = 3;

    if max_size >= input.len() {
        return Ok(input.to_string());
    }

    if max_size > ELLIPSIS_LEN {
        let prefix_size = (max_size - ELLIPSIS_LEN + 1) / 2;
        let suffix_size = (max_size - ELLIPSIS_LEN) / 2;
        let prefix = &input[..prefix_size];
        let suffix = &input[input.len() - suffix_size..];
        return Ok(format!("{prefix}{ELLIPSIS}{suffix}"));
    }

    Ok(input[..max_size].to_string())
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
    fn test_shorten_no_change() {
        assert_eq!(shorten_string("short", 20).unwrap(), "short");
    }

    #[test]
    fn test_shorten_below_ellipsis() {
        // when max_size <= 3, falls back to plain truncation
        assert_eq!(shorten_string("hello world", 2).unwrap(), "he");
    }

    #[test]
    fn test_empty_data() {
        let empty_data: Vec<u8> = vec![];
        let hex = Hex::encode(&empty_data).unwrap();
        assert_eq!(hex, "");

        let decoded = Hex::decode("").unwrap();
        assert_eq!(decoded, empty_data);
    }
}
