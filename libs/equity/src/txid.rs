//! Transaction ID (TXID) — 32-byte SHA-256d hash of a transaction.
//!
//! Bitcoin stores and displays a TXID big-endian, but serializes it little-endian
//! in network/disk formats. This type keeps the hash internally in big-endian
//! (display) order; `serialize` and the byte-slice constructor handle the flip.

use crate::{EquityError, Result};

pub const TXID_SIZE: usize = 32;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Txid {
    hash: [u8; TXID_SIZE],
}

impl Txid {
    /// Construct directly from a 32-byte big-endian (display-order) hash.
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != TXID_SIZE {
            return Err(EquityError(format!(
                "TXID hash must be {TXID_SIZE} bytes"
            )));
        }
        let mut hash = [0u8; TXID_SIZE];
        hash.copy_from_slice(data);
        Ok(Self { hash })
    }

    /// Read a TXID from a wire-format little-endian byte stream and advance the
    /// cursor by 32 bytes. Returns `Err` if fewer than 32 bytes are available.
    pub fn deserialize(input: &mut &[u8]) -> Result<Self> {
        if input.len() < TXID_SIZE {
            return Err(EquityError("Not enough bytes for TXID".to_string()));
        }
        let (head, rest) = input.split_at(TXID_SIZE);
        let mut hash = [0u8; TXID_SIZE];
        hash.copy_from_slice(head);
        hash.reverse();
        *input = rest;
        Ok(Self { hash })
    }

    /// Parse a JSON hex string (big-endian display order) into a Txid.
    pub fn from_json(json: &str) -> Result<Self> {
        let s = json.trim().trim_matches('"');
        if s.len() != TXID_SIZE * 2 {
            return Err(EquityError(format!(
                "TXID hex string must be {} characters",
                TXID_SIZE * 2
            )));
        }
        let bytes = hex::decode(s).map_err(|e| EquityError(format!("Invalid hex: {e}")))?;
        Self::from_data(&bytes)
    }

    /// Render as a JSON string literal (with surrounding quotes), display-order hex.
    pub fn to_json(&self) -> String {
        format!("\"{}\"", self.to_hex_be())
    }

    /// Wire-format serialization: 32 bytes, little-endian.
    pub fn serialize(&self) -> Vec<u8> {
        let mut out = self.hash.to_vec();
        out.reverse();
        out
    }

    pub fn hash(&self) -> &[u8] {
        &self.hash
    }

    /// Hex of the raw internal big-endian bytes (i.e. display order).
    pub fn to_hex(&self) -> String {
        hex::encode(self.hash)
    }

    /// Same as `to_hex` since internal storage is already big-endian.
    pub fn to_hex_be(&self) -> String {
        hex::encode(self.hash)
    }
}

impl std::fmt::Display for Txid {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_hex_be())
    }
}

impl std::str::FromStr for Txid {
    type Err = EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_json(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_txid_from_data() {
        let hash_data = [1u8; 32];
        let txid = Txid::from_data(&hash_data).unwrap();
        assert_eq!(txid.hash().len(), 32);
        assert_eq!(txid.hash(), &hash_data);
    }

    #[test]
    fn test_txid_hex_round_trip_non_palindromic() {
        let hash_data: [u8; 32] = std::array::from_fn(|i| i as u8);
        let txid = Txid::from_data(&hash_data).unwrap();

        let hex_be = txid.to_hex_be();
        assert_eq!(hex_be.len(), 64);

        // `serialize` is little-endian — reverse and re-encode should differ from BE.
        let serialized = txid.serialize();
        assert_ne!(hex::encode(&serialized), hex_be);
        let mut reversed = serialized.clone();
        reversed.reverse();
        assert_eq!(hex::encode(reversed), hex_be);
    }

    #[test]
    fn test_txid_from_hex_string() {
        let hex_string = "0101010101010101010101010101010101010101010101010101010101010101";
        let txid: Txid = hex_string.parse().unwrap();
        assert_eq!(txid.to_hex_be(), hex_string);
    }

    #[test]
    fn test_invalid_txid_length() {
        assert!(Txid::from_data(&[1u8; 16]).is_err());
        assert!(Txid::from_data(&[1u8; 33]).is_err());
    }

    #[test]
    fn test_deserialize_reverses_endianness() {
        // Wire-format little-endian: input is the reverse of display order.
        let display: [u8; 32] = std::array::from_fn(|i| i as u8);
        let mut wire: [u8; 32] = display;
        wire.reverse();

        let mut cursor = &wire[..];
        let txid = Txid::deserialize(&mut cursor).unwrap();
        assert!(cursor.is_empty());
        assert_eq!(txid.hash(), &display);
    }

    #[test]
    fn test_json_roundtrip() {
        let bytes: [u8; 32] = std::array::from_fn(|i| (i as u8).wrapping_mul(7));
        let txid = Txid::from_data(&bytes).unwrap();
        let j = txid.to_json();
        let back = Txid::from_json(&j).unwrap();
        assert_eq!(txid, back);
    }
}
