//! Bitcoin private key — 256-bit scalar with optional "compressed" hint.
//!
//! Accepts construction from:
//!   * 32 raw bytes,
//!   * a WIF (Base58Check) string with optional 0x01 compression suffix,
//!   * a 30-character mini-key (`S...` form). The check is SHA-256 of the
//!     string with a trailing `?` having a zero first byte.

use crate::{EquityError, Result, base58_check};
use sha2::{Digest, Sha256};

pub const PRIVATE_KEY_SIZE: usize = 32;
const COMPRESSED_FLAG: u8 = 0x01;
const MINI_KEY_LEN: usize = 30;

#[derive(Clone)]
pub struct PrivateKey {
    value: [u8; PRIVATE_KEY_SIZE],
    valid: bool,
    compressed: bool,
}

impl PrivateKey {
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != PRIVATE_KEY_SIZE {
            return Err(EquityError(format!(
                "Private key data must be {PRIVATE_KEY_SIZE} bytes"
            )));
        }
        let mut value = [0u8; PRIVATE_KEY_SIZE];
        value.copy_from_slice(data);
        Ok(Self {
            value,
            valid: true,
            compressed: false,
        })
    }

    pub fn from_string(s: &str) -> Result<Self> {
        if s.len() == MINI_KEY_LEN {
            return Self::from_mini_key(s);
        }
        Self::from_wif(s)
    }

    fn from_mini_key(s: &str) -> Result<Self> {
        if !s.starts_with('S') {
            return Err(EquityError("Mini-key must begin with 'S'".to_string()));
        }
        let mut check_input = s.as_bytes().to_vec();
        check_input.push(b'?');
        let check = Sha256::digest(&check_input);
        if check[0] != 0 {
            return Err(EquityError("Mini-key failed type check".to_string()));
        }
        let value: [u8; PRIVATE_KEY_SIZE] = Sha256::digest(s.as_bytes()).into();
        Ok(Self {
            value,
            valid: true,
            compressed: false,
        })
    }

    fn from_wif(s: &str) -> Result<Self> {
        let (mut decoded, _version) = base58_check::decode(s)?;
        let compressed = if decoded.len() == PRIVATE_KEY_SIZE + 1
            && *decoded.last().unwrap() == COMPRESSED_FLAG
        {
            decoded.pop();
            true
        } else if decoded.len() == PRIVATE_KEY_SIZE {
            false
        } else {
            return Err(EquityError(format!(
                "WIF payload must be {PRIVATE_KEY_SIZE} or {} bytes",
                PRIVATE_KEY_SIZE + 1
            )));
        };
        let mut value = [0u8; PRIVATE_KEY_SIZE];
        value.copy_from_slice(&decoded);
        Ok(Self {
            value,
            valid: true,
            compressed,
        })
    }

    pub fn value(&self) -> Vec<u8> {
        self.value.to_vec()
    }

    pub fn value_array(&self) -> &[u8; PRIVATE_KEY_SIZE] {
        &self.value
    }

    pub fn is_valid(&self) -> bool {
        self.valid
    }

    pub fn is_compressed(&self) -> bool {
        self.compressed
    }

    pub fn set_compressed(&mut self, compressed: bool) {
        self.compressed = compressed;
    }

    pub fn to_wif(&self, version: u32) -> String {
        if !self.valid {
            return String::new();
        }
        if self.compressed {
            let mut extended = self.value.to_vec();
            extended.push(COMPRESSED_FLAG);
            base58_check::encode(&extended, version)
        } else {
            base58_check::encode(&self.value, version)
        }
    }

    pub fn to_hex(&self) -> String {
        if !self.valid {
            String::new()
        } else {
            hex::encode(self.value)
        }
    }
}

impl std::fmt::Debug for PrivateKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("PrivateKey")
            .field("valid", &self.valid)
            .field("compressed", &self.compressed)
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_private_key_from_data() {
        let key_data = [1u8; PRIVATE_KEY_SIZE];
        let pk = PrivateKey::from_data(&key_data).unwrap();
        assert!(pk.is_valid());
        assert_eq!(pk.value().len(), PRIVATE_KEY_SIZE);
        assert!(!pk.is_compressed());
    }

    #[test]
    fn test_invalid_private_key_length() {
        assert!(PrivateKey::from_data(&[1u8; 16]).is_err());
        assert!(PrivateKey::from_data(&[1u8; 33]).is_err());
    }

    #[test]
    fn test_wif_round_trip_uncompressed() {
        let key_data = {
            let mut k = [0u8; PRIVATE_KEY_SIZE];
            k[31] = 1;
            k
        };
        let pk = PrivateKey::from_data(&key_data).unwrap();
        let wif = pk.to_wif(0x80);
        assert_eq!(wif, "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf");

        let parsed = PrivateKey::from_string(&wif).unwrap();
        assert_eq!(parsed.value(), pk.value());
        assert!(!parsed.is_compressed());
    }

    #[test]
    fn test_wif_round_trip_compressed() {
        let mut pk = PrivateKey::from_data(&[7u8; PRIVATE_KEY_SIZE]).unwrap();
        pk.set_compressed(true);
        let wif = pk.to_wif(0x80);
        let parsed = PrivateKey::from_string(&wif).unwrap();
        assert!(parsed.is_compressed());
        assert_eq!(parsed.value(), pk.value());
    }

    #[test]
    fn test_to_hex() {
        let pk = PrivateKey::from_data(&[0xABu8; PRIVATE_KEY_SIZE]).unwrap();
        assert_eq!(pk.to_hex().len(), PRIVATE_KEY_SIZE * 2);
        assert!(pk.to_hex().chars().all(|c| c == 'a' || c == 'b'));
    }

    #[test]
    fn test_private_key_display() {
        let pk = PrivateKey::from_data(&[1u8; PRIVATE_KEY_SIZE]).unwrap();
        let dbg = format!("{pk:?}");
        assert!(dbg.contains("valid: true"));
    }
}
