//! Bitcoin private key — 256-bit scalar with optional "compressed" hint.
//!
//! Accepts construction from:
//!   * 32 raw bytes,
//!   * a WIF (Base58Check) string with optional 0x01 compression suffix,
//!   * a 30-character mini-key (`S...` form). The check is SHA-256 of the
//!     string with a trailing `?` having a zero first byte.

use crate::{EquityError, Result, base58_check};
use sha2::{Digest, Sha256};

/// Length of a Bitcoin private key in bytes.
pub const PRIVATE_KEY_SIZE: usize = 32;
const COMPRESSED_FLAG: u8 = 0x01;
const MINI_KEY_LEN: usize = 30;

/// Bitcoin private key (256-bit scalar) with a compression hint.
#[derive(Clone)]
pub struct PrivateKey {
    value: [u8; PRIVATE_KEY_SIZE],
    compressed: bool,
}

impl PrivateKey {
    /// Construct from 32 raw bytes. The compression hint defaults to `false`.
    ///
    /// # Examples
    ///
    /// ```
    /// use equity::private_key::PrivateKey;
    ///
    /// let sk = PrivateKey::from_data(&[1u8; 32]).unwrap();
    /// assert!(!sk.is_compressed());
    /// ```
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let value: [u8; PRIVATE_KEY_SIZE] = data.try_into().map_err(|_| {
            EquityError(format!("Private key data must be {PRIVATE_KEY_SIZE} bytes"))
        })?;
        Ok(Self {
            value,
            compressed: false,
        })
    }

    /// Parse from a WIF string, or from a 30-character mini-key when `s.len() == 30`.
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
        if Sha256::digest(&check_input)[0] != 0 {
            return Err(EquityError("Mini-key failed type check".to_string()));
        }
        Ok(Self {
            value: Sha256::digest(s.as_bytes()).into(),
            compressed: false,
        })
    }

    fn from_wif(s: &str) -> Result<Self> {
        let (mut decoded, _version) = base58_check::decode(s)?;
        let compressed = match decoded.len() {
            n if n == PRIVATE_KEY_SIZE + 1 && *decoded.last().unwrap() == COMPRESSED_FLAG => {
                decoded.pop();
                true
            }
            PRIVATE_KEY_SIZE => false,
            _ => {
                return Err(EquityError(format!(
                    "WIF payload must be {PRIVATE_KEY_SIZE} or {} bytes",
                    PRIVATE_KEY_SIZE + 1
                )));
            }
        };
        Ok(Self {
            value: decoded.as_slice().try_into().unwrap(),
            compressed,
        })
    }

    /// Raw 32-byte scalar as an owned `Vec`.
    pub fn value(&self) -> Vec<u8> {
        self.value.to_vec()
    }

    /// Raw 32-byte scalar as a borrowed fixed-size array.
    pub fn value_array(&self) -> &[u8; PRIVATE_KEY_SIZE] {
        &self.value
    }

    /// True if WIF encoding should mark the derived public key as compressed.
    pub fn is_compressed(&self) -> bool {
        self.compressed
    }

    /// Set the compressed-pubkey flag.
    pub fn set_compressed(&mut self, compressed: bool) {
        self.compressed = compressed;
    }

    /// Encode as a Base58Check WIF string with the given version byte.
    ///
    /// If `is_compressed()` is `true`, the encoded payload has a `0x01` byte
    /// appended (33 bytes total).
    ///
    /// # Examples
    ///
    /// ```
    /// use equity::private_key::PrivateKey;
    ///
    /// let mut sk = [0u8; 32];
    /// sk[31] = 1;
    /// let pk = PrivateKey::from_data(&sk).unwrap();
    /// assert_eq!(pk.to_wif(0x80), "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf");
    /// ```
    pub fn to_wif(&self, version: u8) -> String {
        if self.compressed {
            let mut extended = self.value.to_vec();
            extended.push(COMPRESSED_FLAG);
            base58_check::encode(&extended, version)
        } else {
            base58_check::encode(&self.value, version)
        }
    }

    /// Lower-case hex encoding of the raw scalar.
    pub fn to_hex(&self) -> String {
        hex::encode(self.value)
    }
}

impl std::fmt::Debug for PrivateKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("PrivateKey")
            .field("compressed", &self.compressed)
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn from_data_yields_uncompressed() {
        let pk = PrivateKey::from_data(&[1u8; PRIVATE_KEY_SIZE]).unwrap();
        assert_eq!(pk.value().len(), PRIVATE_KEY_SIZE);
        assert!(!pk.is_compressed());
    }

    #[test]
    fn from_data_rejects_wrong_length() {
        assert!(PrivateKey::from_data(&[1u8; 16]).is_err());
        assert!(PrivateKey::from_data(&[1u8; 33]).is_err());
    }

    #[test]
    fn wif_round_trip_uncompressed() {
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
    fn wif_round_trip_compressed() {
        let mut pk = PrivateKey::from_data(&[7u8; PRIVATE_KEY_SIZE]).unwrap();
        pk.set_compressed(true);
        let wif = pk.to_wif(0x80);
        let parsed = PrivateKey::from_string(&wif).unwrap();
        assert!(parsed.is_compressed());
        assert_eq!(parsed.value(), pk.value());
    }

    #[test]
    fn to_hex_full_length() {
        let pk = PrivateKey::from_data(&[0xABu8; PRIVATE_KEY_SIZE]).unwrap();
        assert_eq!(pk.to_hex().len(), PRIVATE_KEY_SIZE * 2);
        assert!(pk.to_hex().chars().all(|c| c == 'a' || c == 'b'));
    }

    #[test]
    fn debug_format() {
        let pk = PrivateKey::from_data(&[1u8; PRIVATE_KEY_SIZE]).unwrap();
        let dbg = format!("{pk:?}");
        assert!(dbg.contains("compressed"));
    }

    #[test]
    fn wif_known_compressed_vector() {
        let mut k = [0u8; PRIVATE_KEY_SIZE];
        k[31] = 1;
        let mut pk = PrivateKey::from_data(&k).unwrap();
        pk.set_compressed(true);
        let wif = pk.to_wif(0x80);
        assert_eq!(wif, "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn");
    }

    #[test]
    fn invalid_wif_rejected() {
        assert!(PrivateKey::from_string("not a real wif").is_err());
    }

    #[test]
    fn mini_key_rejects_non_s_prefix() {
        let s = "X".repeat(30);
        assert!(PrivateKey::from_string(&s).is_err());
    }

    #[test]
    fn wif_invalid_payload_length() {
        let bogus = base58_check::encode(&[0u8; 20], 0x80);
        assert!(PrivateKey::from_string(&bogus).is_err());
    }

    proptest::proptest! {
        #[test]
        fn prop_wif_round_trip(bytes: [u8; PRIVATE_KEY_SIZE], compressed: bool) {
            let mut pk = PrivateKey::from_data(&bytes).unwrap();
            pk.set_compressed(compressed);
            let wif = pk.to_wif(0x80);
            let back = PrivateKey::from_string(&wif).unwrap();
            proptest::prop_assert_eq!(back.value(), pk.value());
            proptest::prop_assert_eq!(back.is_compressed(), compressed);
        }
    }
}
