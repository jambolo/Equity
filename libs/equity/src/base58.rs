//! Base58 encoding and decoding (Bitcoin alphabet) via the `bs58` crate.

use crate::{EquityError, Result};

pub fn encode(input: &[u8]) -> String {
    bs58::encode(input).into_string()
}

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
}
