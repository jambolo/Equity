//! Base58Check (version byte + 4-byte SHA256d checksum) on top of the `bs58` crate.

use crate::{EquityError, Result};
use sha2::{Digest, Sha256};

fn checksum(data: &[u8]) -> [u8; 4] {
    let first = Sha256::digest(data);
    let second = Sha256::digest(first);
    [second[0], second[1], second[2], second[3]]
}

pub fn encode(input: &[u8], version: u32) -> String {
    let mut buf = Vec::with_capacity(1 + input.len() + 4);
    buf.push(version as u8);
    buf.extend_from_slice(input);
    let check = checksum(&buf);
    buf.extend_from_slice(&check);
    bs58::encode(buf).into_string()
}

pub fn decode(input: &str) -> Result<(Vec<u8>, u32)> {
    let raw = bs58::decode(input)
        .into_vec()
        .map_err(|e| EquityError(format!("Failed to decode Base58 string: {e}")))?;
    if raw.len() < 1 + 4 {
        return Err(EquityError("Base58Check payload too short".to_string()));
    }
    let (body, tail) = raw.split_at(raw.len() - 4);
    if checksum(body) != tail[..4] {
        return Err(EquityError("Base58Check checksum mismatch".to_string()));
    }
    let version = body[0] as u32;
    Ok((body[1..].to_vec(), version))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_base58check_encode_known_vectors() {
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
    fn test_base58check_round_trip() {
        let cases: &[(u32, &[u8])] = &[
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
    fn test_base58check_decode_known_vectors() {
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
    fn test_base58check_bad_checksum_rejected() {
        // Flip a trailing char — checksum should fail.
        let mut s = String::from("16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM");
        let last = s.pop().unwrap();
        let alt = if last == 'M' { 'N' } else { 'M' };
        s.push(alt);
        assert!(decode(&s).is_err());
    }

    #[test]
    fn test_base58check_versions() {
        for version in [0x00u32, 0x05, 0x80, 0xEF, 0xFF] {
            let data = b"test_data_for_version_testing";
            let encoded = encode(data, version);
            let (decoded, decoded_version) = decode(&encoded).unwrap();
            assert_eq!(&decoded[..], data);
            assert_eq!(decoded_version, version);
        }
    }
}
