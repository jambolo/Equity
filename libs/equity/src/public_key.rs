//! Bitcoin public key — secp256k1 point in ANSI X9.62 (compressed or
//! uncompressed) form.

use crate::private_key::PrivateKey;
use crate::{EquityError, Result};
use crypto::ecc;

pub const COMPRESSED_SIZE: usize = ecc::COMPRESSED_PUBLIC_KEY_SIZE; // 33
pub const UNCOMPRESSED_SIZE: usize = ecc::UNCOMPRESSED_PUBLIC_KEY_SIZE; // 65

#[derive(Clone)]
pub struct PublicKey {
    value: Vec<u8>,
    valid: bool,
    compressed: bool,
}

impl PublicKey {
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

    pub fn from_private_key_bytes(private_key_data: &[u8]) -> Result<Self> {
        let pk = PrivateKey::from_data(private_key_data)?;
        Self::from_private_key(&pk)
    }

    pub fn value(&self) -> Vec<u8> {
        self.value.clone()
    }

    pub fn as_bytes(&self) -> &[u8] {
        &self.value
    }

    pub fn is_valid(&self) -> bool {
        self.valid
    }

    pub fn is_compressed(&self) -> bool {
        self.compressed
    }

    pub fn size(&self) -> usize {
        self.value.len()
    }

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
}
