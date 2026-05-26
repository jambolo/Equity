//! Base58 encoding and decoding (Bitcoin alphabet) via the `bs58` crate.

use crate::{EquityError, Result};

/// Encode `input` as a Bitcoin-alphabet Base58 string.
///
/// Each leading zero byte becomes one leading `'1'` character (the Bitcoin
/// convention for preserving prefix information).
///
/// # Examples
///
/// ```
/// use equity::base58;
///
/// assert_eq!(base58::encode(&[0u8; 4]), "1111");
/// ```
pub fn encode(input: &[u8]) -> String {
    bs58::encode(input).into_string()
}

/// Decode a Bitcoin-alphabet Base58 string. Errors on characters outside the
/// alphabet (`0`, `O`, `I`, `l` are not part of the Bitcoin alphabet).
///
/// # Examples
///
/// ```
/// use equity::base58;
///
/// assert_eq!(base58::decode("1111").unwrap(), vec![0u8; 4]);
/// assert!(base58::decode("0").is_err());
/// ```
pub fn decode(input: &str) -> Result<Vec<u8>> {
    bs58::decode(input)
        .into_vec()
        .map_err(|e| EquityError(format!("Failed to decode Base58 string: {e}")))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58_encode_known_vectors() {
        // Bitcoin Base58 preserves one '1' per leading zero byte.
        assert_eq!(encode(&[0u8]), "1");
        assert_eq!(encode(&[0xffu8]), "5Q");
        assert_eq!(encode(&[0u8; 20]), "1".repeat(20));
        assert_eq!(encode(&[0xffu8; 20]), "4ZrjxJnU1LA5xSyrWMNuXTvSYKwt");
    }

    #[test]
    fn test_base58_round_trip() {
        let cases: &[&[u8]] = &[
            &[0],
            &[0xff],
            &[0; 20],
            &[0xff; 20],
            &[0x01, 0x02, 0x03],
            &(0u8..100).collect::<Vec<_>>(),
        ];
        for data in cases {
            let encoded = encode(data);
            let decoded = decode(&encoded).expect("round trip");
            assert_eq!(&decoded[..], *data);
        }
    }

    #[test]
    fn test_base58_decode_invalid() {
        // Chars outside the Bitcoin alphabet must fail.
        for s in ["0", "O", "I", "l"] {
            assert!(decode(s).is_err(), "expected failure decoding {s:?}");
        }
    }

    #[test]
    fn test_base58_empty() {
        assert_eq!(encode(&[]), "");
        assert_eq!(decode("").unwrap(), Vec::<u8>::new());
    }

    proptest::proptest! {
        #[test]
        fn prop_base58_round_trip(data in proptest::collection::vec(any::<u8>(), 0..200)) {
            let s = encode(&data);
            let back = decode(&s).unwrap();
            proptest::prop_assert_eq!(back, data);
        }

        #[test]
        fn prop_base58_leading_zeros_preserved(zeros in 0usize..16, tail in proptest::collection::vec(any::<u8>(), 0..32)) {
            let mut input = vec![0u8; zeros];
            input.extend_from_slice(&tail);
            let s = encode(&input);
            let expected_prefix = "1".repeat(zeros);
            proptest::prop_assert!(s.starts_with(&expected_prefix));
        }
    }

    use proptest::prelude::any;
}
