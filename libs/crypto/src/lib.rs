//! Bitcoin-flavoured cryptographic primitives.
//!
//! Thin wrappers over well-known Rust crates ([`sha1`], [`sha2`], [`ripemd`],
//! [`hmac`], [`pbkdf2`], [`secp256k1`], and [`getrandom`]) presented behind a
//! single namespace with the shapes the rest of the Equity workspace needs:
//!
//! - [`sha1`] / [`sha256`] / [`sha512`] — fixed-size hash digests, plus
//!   Bitcoin's [`sha256::double_sha256`] and 4-byte [`sha256::checksum`].
//! - [`ripemd`] — RIPEMD-160 used for HASH160 address derivation.
//! - [`hmac`] — HMAC-SHA-512 (used by BIP-39 / BIP-32).
//! - [`pbkdf2`] — PBKDF2-HMAC-SHA-512 (used by BIP-39 seed derivation).
//! - [`ecc`] — secp256k1 key derivation, ECDSA sign/verify, validation.
//! - [`random`] — OS-backed CSPRNG via `getrandom`.
//!
//! Everything in this crate operates on borrowed byte slices and returns
//! fixed-size arrays where the digest size is known.
//!
//! # Examples
//!
//! Bitcoin checksum:
//!
//! ```
//! use crypto::sha256;
//!
//! let c = sha256::checksum(b"");
//! assert_eq!(c, [0x5d, 0xf6, 0xe0, 0xe2]);
//! ```
//!
//! ECDSA sign and verify:
//!
//! ```
//! use crypto::ecc::{derive_public_key, sign, verify};
//!
//! let mut sk = [0u8; 32];
//! sk[31] = 1;
//! let pk = derive_public_key(&sk, true).unwrap();
//! let sig = sign(b"hello", &sk).unwrap();
//! assert!(verify(b"hello", &pk, &sig));
//! ```

pub mod ecc;
pub mod hmac;
pub mod pbkdf2;
pub mod random;
pub mod ripemd;
pub mod sha1;
pub mod sha256;
pub mod sha512;

pub use ecc::*;
pub use hmac::*;
pub use pbkdf2::*;
pub use random::*;
pub use ripemd::*;
pub use sha1::*;
pub use sha256::*;
pub use sha512::*;
