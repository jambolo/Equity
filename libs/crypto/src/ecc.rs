//! ECC over secp256k1 via the `secp256k1` Rust crate.

use secp256k1::{Message, PublicKey as SecpPublicKey, SECP256K1, SecretKey, ecdsa::Signature};
use sha2::{Digest, Sha256};

pub type PrivateKey = [u8; 32];
pub type PublicKey = Vec<u8>;
pub type SignatureBytes = Vec<u8>;

pub const PRIVATE_KEY_SIZE: usize = 32;
pub const COMPRESSED_PUBLIC_KEY_SIZE: usize = 33;
pub const UNCOMPRESSED_PUBLIC_KEY_SIZE: usize = 65;

pub fn public_key_is_valid(key: &[u8]) -> bool {
    SecpPublicKey::from_slice(key).is_ok()
}

pub fn private_key_is_valid(key: &PrivateKey) -> bool {
    SecretKey::from_slice(key).is_ok()
}

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

pub fn sign(message: &[u8], private_key: &PrivateKey) -> Result<SignatureBytes, &'static str> {
    let sk = SecretKey::from_slice(private_key).map_err(|_| "invalid private key")?;
    let sig = SECP256K1.sign_ecdsa(&message_digest(message), &sk);
    Ok(sig.serialize_der().to_vec())
}

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
