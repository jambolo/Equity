//! ECC over secp256k1 via the `secp256k1` Rust crate.

use secp256k1::{Message, PublicKey as SecpPublicKey, SECP256K1, SecretKey, ecdsa::Signature};
use sha2::{Digest, Sha256};

/// Raw secp256k1 scalar.
pub type PrivateKey = [u8; 32];
/// Serialized secp256k1 point (compressed or uncompressed).
pub type PublicKey = Vec<u8>;
/// DER-encoded ECDSA signature.
pub type SignatureBytes = Vec<u8>;

/// Length of a secp256k1 scalar in bytes.
pub const PRIVATE_KEY_SIZE: usize = 32;
/// Length of a compressed secp256k1 public key.
pub const COMPRESSED_PUBLIC_KEY_SIZE: usize = 33;
/// Length of an uncompressed secp256k1 public key.
pub const UNCOMPRESSED_PUBLIC_KEY_SIZE: usize = 65;

/// True if `key` decodes as a valid secp256k1 public key.
pub fn public_key_is_valid(key: &[u8]) -> bool {
    SecpPublicKey::from_slice(key).is_ok()
}

/// True if `key` is a valid secp256k1 scalar (nonzero and below the group order).
pub fn private_key_is_valid(key: &PrivateKey) -> bool {
    SecretKey::from_slice(key).is_ok()
}

/// Derive the secp256k1 public key for `private_key`.
///
/// Set `uncompressed = true` for the 65-byte uncompressed form, or `false`
/// for the 33-byte compressed form.
///
/// Returns `Err` if `private_key` is not a valid scalar (zero or above the
/// group order).
///
/// # Examples
///
/// ```
/// use crypto::ecc::{derive_public_key, COMPRESSED_PUBLIC_KEY_SIZE, UNCOMPRESSED_PUBLIC_KEY_SIZE};
///
/// let mut sk = [0u8; 32];
/// sk[31] = 1;
/// let compressed = derive_public_key(&sk, false).unwrap();
/// let uncompressed = derive_public_key(&sk, true).unwrap();
/// assert_eq!(compressed.len(), COMPRESSED_PUBLIC_KEY_SIZE);
/// assert_eq!(uncompressed.len(), UNCOMPRESSED_PUBLIC_KEY_SIZE);
/// ```
pub fn derive_public_key(
    private_key: &PrivateKey,
    uncompressed: bool,
) -> Result<PublicKey, &'static str> {
    let sk = SecretKey::from_slice(private_key).map_err(|_| "invalid private key")?;
    let pk = SecpPublicKey::from_secret_key(SECP256K1, &sk);
    Ok(if uncompressed {
        pk.serialize_uncompressed().to_vec()
    } else {
        pk.serialize().to_vec()
    })
}

fn message_digest(message: &[u8]) -> Message {
    let digest: [u8; 32] = Sha256::digest(message).into();
    Message::from_digest(digest)
}

/// Sign SHA-256(`message`) with `private_key`. Returns a DER-encoded ECDSA signature.
///
/// # Examples
///
/// ```
/// use crypto::ecc::{sign, derive_public_key, verify};
///
/// let mut sk = [0u8; 32];
/// sk[31] = 1;
/// let pk = derive_public_key(&sk, true).unwrap();
/// let sig = sign(b"hello", &sk).unwrap();
/// assert!(verify(b"hello", &pk, &sig));
/// ```
pub fn sign(message: &[u8], private_key: &PrivateKey) -> Result<SignatureBytes, &'static str> {
    let sk = SecretKey::from_slice(private_key).map_err(|_| "invalid private key")?;
    let sig = SECP256K1.sign_ecdsa(&message_digest(message), &sk);
    Ok(sig.serialize_der().to_vec())
}

/// Verify a DER-encoded ECDSA `signature` over SHA-256(`message`) against `public_key`.
pub fn verify(message: &[u8], public_key: &PublicKey, signature: &SignatureBytes) -> bool {
    let Ok(pk) = SecpPublicKey::from_slice(public_key) else {
        return false;
    };
    let Ok(sig) = Signature::from_der(signature) else {
        return false;
    };
    SECP256K1
        .verify_ecdsa(&message_digest(message), &sig, &pk)
        .is_ok()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn test_key() -> PrivateKey {
        let mut k = [0u8; 32];
        k[31] = 1;
        k
    }

    #[test]
    fn test_private_key_validation() {
        assert!(!private_key_is_valid(&[0u8; 32]));
        assert!(private_key_is_valid(&test_key()));
    }

    #[test]
    fn test_public_key_validation() {
        assert!(!public_key_is_valid(&[0u8; 33]));
        let pk = derive_public_key(&test_key(), false).unwrap();
        assert!(public_key_is_valid(&pk));
    }

    #[test]
    fn test_key_derivation_sizes() {
        let sk = test_key();
        assert_eq!(
            derive_public_key(&sk, false).unwrap().len(),
            COMPRESSED_PUBLIC_KEY_SIZE
        );
        assert_eq!(
            derive_public_key(&sk, true).unwrap().len(),
            UNCOMPRESSED_PUBLIC_KEY_SIZE
        );
    }

    #[test]
    fn test_signing_and_verification() {
        let sk = test_key();
        let pk = derive_public_key(&sk, true).unwrap();
        let message = b"test message";
        let signature = sign(message, &sk).unwrap();
        assert!(verify(message, &pk, &signature));
        assert!(!verify(b"different message", &pk, &signature));
    }
}
