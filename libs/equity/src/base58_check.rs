//! Base58Check (version byte + 4-byte SHA256d checksum) on top of the `bs58` crate.

use crate::{EquityError, Result};
use crypto::sha256;

/// Encode `input` as Base58Check with a single-byte `version` prefix.
///
/// Common values are `0x00` (mainnet P2PKH), `0x05` (mainnet P2SH),
/// `0x80` (mainnet WIF), and `0x6F` (testnet P2PKH).
///
/// # Examples
///
/// ```
/// use equity::base58_check;
///
/// let s = base58_check::encode(&[0u8; 20], 0x00);
/// assert!(s.starts_with('1'));
/// ```
pub fn encode(input: &[u8], version: u8) -> String {
    let mut buf = Vec::with_capacity(1 + input.len() + sha256::CHECKSUM_SIZE);
    buf.push(version);
    buf.extend_from_slice(input);
    buf.extend_from_slice(&sha256::checksum(&buf));
    bs58::encode(buf).into_string()
}

/// Decode a Base58Check string into `(payload, version)`. Verifies the 4-byte checksum.
///
/// # Examples
///
/// ```
/// use equity::base58_check;
///
/// let s = base58_check::encode(b"hello", 0x05);
/// let (payload, version) = base58_check::decode(&s).unwrap();
/// assert_eq!(payload, b"hello");
/// assert_eq!(version, 0x05);
/// ```
pub fn decode(input: &str) -> Result<(Vec<u8>, u8)> {
    let raw = bs58::decode(input)
        .into_vec()
        .map_err(|e| EquityError(format!("Failed to decode Base58 string: {e}")))?;
    if raw.len() < 1 + sha256::CHECKSUM_SIZE {
        return Err(EquityError("Base58Check payload too short".to_string()));
    }
    let (body, tail) = raw.split_at(raw.len() - sha256::CHECKSUM_SIZE);
    if sha256::checksum(body) != tail {
        return Err(EquityError("Base58Check checksum mismatch".to_string()));
    }
    Ok((body[1..].to_vec(), body[0]))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn encode_matches_known_vectors() {
        assert_eq!(encode(&[0u8; 20], 0), "1111111111111111111114oLvT2");

        let addr_data = [
            0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39,
            0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE,
        ];
        assert_eq!(encode(&addr_data, 0), "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM");

        assert_eq!(
            encode(&[0xFFu8; 20], 0),
            "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr"
        );

        let priv_one = [
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 1,
        ];
        assert_eq!(
            encode(&priv_one, 0x80),
            "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf"
        );
    }

    #[test]
    fn round_trip() {
        let cases: &[(u8, &[u8])] = &[
            (0, &[0u8; 20]),
            (0x80, &[0u8; 32]),
            (0x05, b"test_data"),
            (0x42, &(0u8..100).collect::<Vec<_>>()),
        ];
        for (version, data) in cases {
            let encoded = encode(data, *version);
            let (decoded, decoded_version) = decode(&encoded).expect("round trip");
            assert_eq!(&decoded[..], *data);
            assert_eq!(decoded_version, *version);
        }
    }

    #[test]
    fn decode_known_vectors() {
        let (data, version) = decode("1111111111111111111114oLvT2").unwrap();
        assert_eq!(version, 0);
        assert_eq!(data, vec![0u8; 20]);

        let (data, version) =
            decode("5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf").unwrap();
        assert_eq!(version, 0x80);
        assert_eq!(data.len(), 32);
        assert_eq!(data[31], 1);
    }

    #[test]
    fn bad_checksum_rejected() {
        let mut s = String::from("16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM");
        let last = s.pop().unwrap();
        let alt = if last == 'M' { 'N' } else { 'M' };
        s.push(alt);
        assert!(decode(&s).is_err());
    }

    #[test]
    fn versions() {
        for version in [0x00u8, 0x05, 0x80, 0xEF, 0xFF] {
            let data = b"test_data_for_version_testing";
            let encoded = encode(data, version);
            let (decoded, decoded_version) = decode(&encoded).unwrap();
            assert_eq!(&decoded[..], data);
            assert_eq!(decoded_version, version);
        }
    }

    #[test]
    fn p2sh_vector() {
        let h160 = [
            0x74, 0xf2, 0x09, 0xf6, 0xea, 0x90, 0x7e, 0x2e, 0xa4, 0x8f, 0x74, 0xfa, 0xe0, 0x5d,
            0xd0, 0x6c, 0xae, 0xa1, 0x86, 0x05,
        ];
        let s = encode(&h160, 0x05);
        assert!(s.starts_with('3'));
        let (back, v) = decode(&s).unwrap();
        assert_eq!(back, h160);
        assert_eq!(v, 0x05);
    }

    #[test]
    fn decode_too_short() {
        let s = bs58::encode([0u8; 3]).into_string();
        assert!(decode(&s).is_err());
    }

    proptest::proptest! {
        #[test]
        fn prop_round_trip(
            version: u8,
            data in proptest::collection::vec(any::<u8>(), 0..64),
        ) {
            let encoded = encode(&data, version);
            let (decoded, decoded_version) = decode(&encoded).unwrap();
            proptest::prop_assert_eq!(decoded, data);
            proptest::prop_assert_eq!(decoded_version, version);
        }
    }

    use proptest::prelude::any;
}
