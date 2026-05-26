//! PBKDF2-HMAC-SHA512 via the `pbkdf2` + `hmac` + `sha2` Rust crates.

use anyhow::{Result, anyhow, bail};
use hmac::Hmac;
use sha2::Sha512;

/// Derive `output_size` bytes via PBKDF2-HMAC-SHA-512 with the given iteration `count`.
///
/// # Examples
///
/// ```
/// use crypto::pbkdf2::pbkdf2_hmac_sha512;
///
/// let key = pbkdf2_hmac_sha512(b"password", b"salt", 1000, 32).unwrap();
/// assert_eq!(key.len(), 32);
/// ```
pub fn pbkdf2_hmac_sha512(
    password: &[u8],
    salt: &[u8],
    count: u32,
    output_size: usize,
) -> Result<Vec<u8>> {
    if count == 0 {
        bail!("PBKDF2 iteration count must be positive");
    }
    let mut output = vec![0u8; output_size];
    pbkdf2::pbkdf2::<Hmac<Sha512>>(password, salt, count, &mut output)
        .map_err(|e| anyhow!("PBKDF2 failed: {e}"))?;
    Ok(output)
}

#[cfg(test)]
mod tests {
    use super::*;

    struct Pbkdf2HmacSha512TestCase {
        password: &'static str,
        salt: &'static str,
        count: u32,
        expected: [u8; 64],
    }

    const PBKDF2_HMAC_SHA512_CASES: &[Pbkdf2HmacSha512TestCase] = &[
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTT",
            salt: "saltKEYbcTcXHCBxtjD",
            count: 1,
            expected: [
                0xCB, 0xE6, 0x08, 0x8A, 0xD4, 0x35, 0x9A, 0xF4, 0x2E, 0x60, 0x3C, 0x2A, 0x33, 0x76,
                0x0E, 0xF9, 0xD4, 0x01, 0x7A, 0x7B, 0x2A, 0xAD, 0x10, 0xAF, 0x46, 0xF9, 0x92, 0xC6,
                0x60, 0xA0, 0xB4, 0x61, 0xEC, 0xB0, 0xDC, 0x2A, 0x79, 0xC2, 0x57, 0x09, 0x41, 0xBE,
                0xA6, 0xA0, 0x8D, 0x15, 0xD6, 0x88, 0x7E, 0x79, 0xF3, 0x2B, 0x13, 0x2E, 0x1C, 0x13,
                0x4E, 0x95, 0x25, 0xEE, 0xDD, 0xD7, 0x44, 0xFA,
            ],
        },
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTT",
            salt: "saltKEYbcTcXHCBxtjD",
            count: 100000,
            expected: [
                0xAC, 0xCD, 0xCD, 0x87, 0x98, 0xAE, 0x5C, 0xD8, 0x58, 0x04, 0x73, 0x90, 0x15, 0xEF,
                0x2A, 0x11, 0xE3, 0x25, 0x91, 0xB7, 0xB7, 0xD1, 0x6F, 0x76, 0x81, 0x9B, 0x30, 0xB0,
                0xD4, 0x9D, 0x80, 0xE1, 0xAB, 0xEA, 0x6C, 0x98, 0x22, 0xB8, 0x0A, 0x1F, 0xDF, 0xE4,
                0x21, 0xE2, 0x6F, 0x56, 0x03, 0xEC, 0xA8, 0xA4, 0x7A, 0x64, 0xC9, 0xA0, 0x04, 0xFB,
                0x5A, 0xF8, 0x22, 0x9F, 0x76, 0x2F, 0xF4, 0x1F,
            ],
        },
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTTl",
            salt: "saltKEYbcTcXHCBxtjD2",
            count: 1,
            expected: [
                0x8E, 0x50, 0x74, 0xA9, 0x51, 0x3C, 0x1F, 0x15, 0x12, 0xC9, 0xB1, 0xDF, 0x1D, 0x8B,
                0xFF, 0xA9, 0xD8, 0xB4, 0xEF, 0x91, 0x05, 0xDF, 0xC1, 0x66, 0x81, 0x22, 0x28, 0x39,
                0x56, 0x0F, 0xB6, 0x32, 0x64, 0xBE, 0xD6, 0xAA, 0xBF, 0x76, 0x1F, 0x18, 0x0E, 0x91,
                0x2A, 0x66, 0xE0, 0xB5, 0x3D, 0x65, 0xEC, 0x88, 0xF6, 0xA1, 0x51, 0x9E, 0x14, 0x80,
                0x4E, 0xBA, 0x6D, 0xC9, 0xDF, 0x13, 0x70, 0x07,
            ],
        },
    ];

    #[test]
    fn matches_known_vectors() {
        for (i, case) in PBKDF2_HMAC_SHA512_CASES.iter().enumerate() {
            let key = pbkdf2_hmac_sha512(
                case.password.as_bytes(),
                case.salt.as_bytes(),
                case.count,
                64,
            )
            .unwrap_or_else(|e| panic!("PBKDF2 case {} failed: {e}", i + 1));
            let actual: [u8; 64] = key.as_slice().try_into().unwrap();
            assert_eq!(actual, case.expected, "PBKDF2 case {}", i + 1);
        }
    }

    #[test]
    fn output_size_honored() {
        for size in [16usize, 32, 48, 64, 80] {
            let key = pbkdf2_hmac_sha512(b"test_password", b"test_salt", 1000, size).unwrap();
            assert_eq!(key.len(), size);
        }
    }

    #[test]
    fn edge_cases() {
        assert!(pbkdf2_hmac_sha512(b"password", b"salt", 1, 32).is_ok());
        assert!(pbkdf2_hmac_sha512(&[], b"salt", 1000, 32).is_ok());
        assert!(pbkdf2_hmac_sha512(b"password", &[], 1000, 32).is_ok());
        assert!(pbkdf2_hmac_sha512(b"password", b"salt", 0, 32).is_err());
    }
}
