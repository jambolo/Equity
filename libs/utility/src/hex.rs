//! Hexadecimal encoding and decoding utilities.

/// Static facade for hex encoding/decoding, with reversed-order variants.
pub struct Hex;

impl Hex {
    /// Encode `data` as lower-case hex.
    ///
    /// # Examples
    ///
    /// ```
    /// use utility::Hex;
    /// assert_eq!(Hex::encode(&[0xDE, 0xAD]).unwrap(), "dead");
    /// ```
    pub fn encode(data: &[u8]) -> Result<String, &'static str> {
        Ok(hex::encode(data))
    }

    /// Decode hex `s` into bytes. Accepts upper- or lower-case; rejects odd
    /// length and non-hex characters.
    ///
    /// # Examples
    ///
    /// ```
    /// use utility::Hex;
    /// assert_eq!(Hex::decode("DEAD").unwrap(), vec![0xDE, 0xAD]);
    /// assert!(Hex::decode("zz").is_err());
    /// ```
    pub fn decode(s: &str) -> Result<Vec<u8>, &'static str> {
        hex::decode(s).map_err(|_| "Failed to decode from hex")
    }

    /// Encode `data` reversed (Bitcoin display-order encoding for hashes).
    ///
    /// # Examples
    ///
    /// ```
    /// use utility::Hex;
    /// assert_eq!(Hex::encode_reversed(&[0x12, 0x34]).unwrap(), "3412");
    /// ```
    pub fn encode_reversed(data: &[u8]) -> Result<String, &'static str> {
        let mut reversed: Vec<u8> = data.iter().rev().copied().collect();
        let out = hex::encode(&reversed);
        reversed.clear();
        Ok(out)
    }

    /// Decode hex `s` and reverse the byte order.
    pub fn decode_reversed(s: &str) -> Result<Vec<u8>, &'static str> {
        let mut bytes = hex::decode(s).map_err(|_| "Failed to decode from reversed hex")?;
        bytes.reverse();
        Ok(bytes)
    }
}

/// Hex-encoding for byte slices and arrays.
pub trait ToHex {
    /// Encode as lower-case hex.
    fn to_hex(&self) -> Result<String, &'static str>;
    /// Encode reversed as lower-case hex (Bitcoin display order).
    fn to_hex_reversed(&self) -> Result<String, &'static str>;
}

/// Hex-decoding into the implementor.
pub trait FromHex: Sized {
    /// Decode `hex` into `Self`.
    fn from_hex(hex: &str) -> Result<Self, &'static str>;
    /// Decode `hex` and reverse the byte order before producing `Self`.
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

/// Shorten `input` to at most `max_size` characters by replacing the middle
/// with `...`.
///
/// When `max_size <= 3` the function falls back to plain truncation. When
/// `max_size >= input.len()` the input is returned unchanged.
///
/// **Note:** This function slices by byte index. Only ASCII input is safe;
/// passing strings containing multi-byte UTF-8 characters may panic.
///
/// # Examples
///
/// ```
/// use utility::shorten_string;
///
/// assert_eq!(shorten_string("hello world", 7).unwrap(), "he...ld");
/// assert_eq!(shorten_string("short", 20).unwrap(), "short");
/// assert_eq!(shorten_string("abcdef", 3).unwrap(), "abc");
/// ```
pub fn shorten_string(input: &str, max_size: usize) -> Result<String, &'static str> {
    const ELLIPSIS: &str = "...";
    const ELLIPSIS_LEN: usize = 3;

    if max_size >= input.len() {
        return Ok(input.to_string());
    }

    if max_size > ELLIPSIS_LEN {
        let prefix_size = (max_size - ELLIPSIS_LEN).div_ceil(2);
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

    #[test]
    fn test_decode_rejects_odd_length() {
        assert!(Hex::decode("abc").is_err());
    }

    #[test]
    fn test_decode_rejects_non_hex_chars() {
        assert!(Hex::decode("zz").is_err());
        assert!(Hex::decode("0g").is_err());
    }

    #[test]
    fn test_decode_uppercase() {
        assert_eq!(Hex::decode("DEADBEEF").unwrap(), vec![0xDE, 0xAD, 0xBE, 0xEF]);
    }

    #[test]
    fn test_shorten_max_size_zero() {
        assert_eq!(shorten_string("hello", 0).unwrap(), "");
    }

    #[test]
    fn test_shorten_equal_to_input_len() {
        assert_eq!(shorten_string("hello", 5).unwrap(), "hello");
    }

    #[test]
    fn test_shorten_just_above_ellipsis() {
        // max_size == 4 → prefix=(4-3+1)/2=1, suffix=(4-3)/2=0
        let out = shorten_string("abcdef", 4).unwrap();
        assert_eq!(out, "a...");
        assert!(out.len() <= 4);
    }

    #[test]
    fn test_shorten_exact_ellipsis_size() {
        // max_size == 3 → falls through to plain truncation
        assert_eq!(shorten_string("abcdef", 3).unwrap(), "abc");
    }

    #[test]
    fn test_from_hex_reversed_vec() {
        let v = Vec::<u8>::from_hex_reversed("cdab3412").unwrap();
        assert_eq!(v, vec![0x12, 0x34, 0xab, 0xcd]);
    }

    proptest::proptest! {
        #[test]
        fn prop_hex_round_trip(data: Vec<u8>) {
            let s = Hex::encode(&data).unwrap();
            let back = Hex::decode(&s).unwrap();
            proptest::prop_assert_eq!(back, data);
        }

        #[test]
        fn prop_hex_reversed_round_trip(data: Vec<u8>) {
            let s = Hex::encode_reversed(&data).unwrap();
            let back = Hex::decode_reversed(&s).unwrap();
            proptest::prop_assert_eq!(back, data);
        }

        #[test]
        fn prop_hex_length_doubles(data: Vec<u8>) {
            let s = Hex::encode(&data).unwrap();
            proptest::prop_assert_eq!(s.len(), data.len() * 2);
        }

        #[test]
        fn prop_shorten_never_exceeds_max(s in "[ -~]{0,200}", max in 0usize..50) {
            let out = shorten_string(&s, max).unwrap();
            // shorten may return original string when max >= input.len()
            proptest::prop_assert!(out.len() <= s.len().max(max));
        }
    }
}
