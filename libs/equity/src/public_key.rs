//! Bitcoin public key — secp256k1 point in ANSI X9.62 (compressed or
//! uncompressed) form.

use crate::private_key::PrivateKey;
use crate::{EquityError, Result};
use crypto::ecc;

/// Length of a compressed secp256k1 public key.
pub const COMPRESSED_SIZE: usize = ecc::COMPRESSED_PUBLIC_KEY_SIZE; // 33
/// Length of an uncompressed secp256k1 public key.
pub const UNCOMPRESSED_SIZE: usize = ecc::UNCOMPRESSED_PUBLIC_KEY_SIZE; // 65

/// Bitcoin public key (secp256k1 point) in compressed or uncompressed form.
#[derive(Clone)]
pub struct PublicKey {
    value: Vec<u8>,
    valid: bool,
    compressed: bool,
}

impl PublicKey {
    /// Construct from serialized public-key bytes, verifying the point is on the curve.
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != COMPRESSED_SIZE && data.len() != UNCOMPRESSED_SIZE {
            return Err(EquityError(format!(
                "Public key must be {COMPRESSED_SIZE} or {UNCOMPRESSED_SIZE} bytes"
            )));
        }
        if !ecc::public_key_is_valid(data) {
            return Err(EquityError("Public key failed ECC validation".to_string()));
        }
        Ok(Self {
            value: data.to_vec(),
            valid: true,
            compressed: data[0] != 4,
        })
    }

    /// Derive the public key for `pk`. Compressed iff `pk.is_compressed()`.
    ///
    /// # Examples
    ///
    /// ```
    /// use equity::{private_key::PrivateKey, public_key::{PublicKey, COMPRESSED_SIZE, UNCOMPRESSED_SIZE}};
    ///
    /// let mut sk = PrivateKey::from_data(&[1u8; 32]).unwrap();
    /// assert_eq!(PublicKey::from_private_key(&sk).unwrap().size(), UNCOMPRESSED_SIZE);
    /// sk.set_compressed(true);
    /// assert_eq!(PublicKey::from_private_key(&sk).unwrap().size(), COMPRESSED_SIZE);
    /// ```
    pub fn from_private_key(pk: &PrivateKey) -> Result<Self> {
        if !pk.is_valid() {
            return Err(EquityError("Source private key is invalid".to_string()));
        }
        let value = ecc::derive_public_key(pk.value_array(), !pk.is_compressed())
            .map_err(|e| EquityError(e.to_string()))?;
        Ok(Self {
            value,
            valid: true,
            compressed: pk.is_compressed(),
        })
    }

    /// Derive the public key from raw 32-byte private-key data (uncompressed).
    pub fn from_private_key_bytes(private_key_data: &[u8]) -> Result<Self> {
        let pk = PrivateKey::from_data(private_key_data)?;
        Self::from_private_key(&pk)
    }

    /// Serialized public-key bytes as an owned `Vec`.
    pub fn value(&self) -> Vec<u8> {
        self.value.clone()
    }

    /// Serialized public-key bytes as a borrowed slice.
    pub fn as_bytes(&self) -> &[u8] {
        &self.value
    }

    /// True if this key was constructed successfully.
    pub fn is_valid(&self) -> bool {
        self.valid
    }

    /// True if encoded in compressed (33-byte) form.
    pub fn is_compressed(&self) -> bool {
        self.compressed
    }

    /// Encoded length in bytes (33 or 65).
    pub fn size(&self) -> usize {
        self.value.len()
    }

    /// Lower-case hex encoding.
    pub fn to_hex(&self) -> String {
        hex::encode(&self.value)
    }
}

impl std::fmt::Debug for PublicKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("PublicKey")
            .field("valid", &self.valid)
            .field("compressed", &self.compressed)
            .field("size", &self.size())
            .finish()
    }
}

impl std::fmt::Display for PublicKey {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_hex())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn one_key() -> PrivateKey {
        let mut k = [0u8; 32];
        k[31] = 1;
        PrivateKey::from_data(&k).unwrap()
    }

    #[test]
    fn test_public_key_from_private_key() {
        let pk = PublicKey::from_private_key(&one_key()).unwrap();
        assert!(pk.is_valid());
        assert_eq!(pk.size(), UNCOMPRESSED_SIZE); // uncompressed by default
        assert!(!pk.is_compressed());
    }

    #[test]
    fn test_public_key_compressed_derivation() {
        let mut sk = one_key();
        sk.set_compressed(true);
        let pk = PublicKey::from_private_key(&sk).unwrap();
        assert_eq!(pk.size(), COMPRESSED_SIZE);
        assert!(pk.is_compressed());
    }

    #[test]
    fn test_invalid_public_key_length() {
        assert!(PublicKey::from_data(&[1u8; 32]).is_err());
        assert!(PublicKey::from_data(&[0u8; 10]).is_err());
    }

    #[test]
    fn test_invalid_public_key_bytes() {
        // Right length, wrong content
        let bad = [0u8; COMPRESSED_SIZE];
        assert!(PublicKey::from_data(&bad).is_err());
    }

    #[test]
    fn test_compressed_vs_uncompressed_round_trip() {
        let sk_bytes = [3u8; 32];
        let pk_u = PublicKey::from_private_key_bytes(&sk_bytes).unwrap();
        assert_eq!(pk_u.size(), UNCOMPRESSED_SIZE);

        let mut sk = PrivateKey::from_data(&sk_bytes).unwrap();
        sk.set_compressed(true);
        let pk_c = PublicKey::from_private_key(&sk).unwrap();
        assert_eq!(pk_c.size(), COMPRESSED_SIZE);

        // Compressed and uncompressed must reduce to the same X coordinate
        assert_eq!(&pk_c.as_bytes()[1..33], &pk_u.as_bytes()[1..33]);
    }

    #[test]
    fn test_from_data_round_trip_uncompressed() {
        let sk = [5u8; 32];
        let pk = PublicKey::from_private_key_bytes(&sk).unwrap();
        let bytes = pk.value();
        let parsed = PublicKey::from_data(&bytes).unwrap();
        assert_eq!(parsed.as_bytes(), pk.as_bytes());
        assert!(!parsed.is_compressed());
    }

    #[test]
    fn test_from_data_round_trip_compressed() {
        let mut sk = PrivateKey::from_data(&[7u8; 32]).unwrap();
        sk.set_compressed(true);
        let pk = PublicKey::from_private_key(&sk).unwrap();
        let parsed = PublicKey::from_data(pk.as_bytes()).unwrap();
        assert_eq!(parsed.as_bytes(), pk.as_bytes());
        assert!(parsed.is_compressed());
    }

    #[test]
    fn test_to_hex_length() {
        let pk = PublicKey::from_private_key_bytes(&[1u8; 32]).unwrap();
        assert_eq!(pk.to_hex().len(), pk.size() * 2);
    }

    proptest::proptest! {
        #[test]
        fn prop_pubkey_serialize_round_trip(seed: [u8; 32], compressed: bool) {
            // secp256k1 SecretKey rejects 0 and values >= group order. Reject those.
            let mut sk = match PrivateKey::from_data(&seed) {
                Ok(k) => k,
                Err(_) => return Ok(()),
            };
            sk.set_compressed(compressed);
            let pk = match PublicKey::from_private_key(&sk) {
                Ok(k) => k,
                Err(_) => return Ok(()),
            };
            let parsed = PublicKey::from_data(pk.as_bytes()).unwrap();
            proptest::prop_assert_eq!(parsed.as_bytes(), pk.as_bytes());
            proptest::prop_assert_eq!(parsed.is_compressed(), compressed);
        }
    }
}
