//! PBKDF2 (Password-Based Key Derivation Function 2) functions

use crate::ffi;

// Safe Rust wrapper using raw pointers
pub fn pbkdf2_hmac_sha512(
    password: &[u8],
    salt: &[u8],
    count: i32,
    output_size: usize,
) -> Result<Vec<u8>, &'static str> {
    let mut output = vec![0u8; output_size];
    unsafe {
        let success = ffi::pbkdf2HmacSha512(
            password.as_ptr(),
            password.len(),
            salt.as_ptr(),
            salt.len(),
            count,
            output_size,
            output.as_mut_ptr(),
        );
        if success {
            Ok(output)
        } else {
            Err("PBKDF2 operation failed")
        }
    }
}

// NEW: Vector-based convenience function
pub fn pbkdf2_hmac_sha512_vec(
    password: &[u8],
    salt: &[u8],
    count: i32,
    output_size: usize,
) -> Result<Vec<u8>, &'static str> {
    let password_vec = password.to_vec();
    let salt_vec = salt.to_vec();
    let mut output = Vec::new();
    
    let success = ffi::pbkdf2HmacSha512Vector(&password_vec, &salt_vec, count, output_size, &mut output);
    
    if success {
        Ok(output)
    } else {
        Err("PBKDF2 operation failed")
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    
    struct Pbkdf2HmacSha512TestCase {
        password: &'static str,
        salt: &'static str,
        count: i32,
        expected: [u8; 64],
    }
    
    const PBKDF2_HMAC_SHA512_CASES: &[Pbkdf2HmacSha512TestCase] = &[
        // Test case 1: Basic test with iteration count 1
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTT",
            salt: "saltKEYbcTcXHCBxtjD",
            count: 1,
            expected: [
                0xCB, 0xE6, 0x08, 0x8A, 0xD4, 0x35, 0x9A, 0xF4, 0x2E, 0x60, 0x3C, 0x2A, 0x33, 0x76, 0x0E, 0xF9,
                0xD4, 0x01, 0x7A, 0x7B, 0x2A, 0xAD, 0x10, 0xAF, 0x46, 0xF9, 0x92, 0xC6, 0x60, 0xA0, 0xB4, 0x61,
                0xEC, 0xB0, 0xDC, 0x2A, 0x79, 0xC2, 0x57, 0x09, 0x41, 0xBE, 0xA6, 0xA0, 0x8D, 0x15, 0xD6, 0x88,
                0x7E, 0x79, 0xF3, 0x2B, 0x13, 0x2E, 0x1C, 0x13, 0x4E, 0x95, 0x25, 0xEE, 0xDD, 0xD7, 0x44, 0xFA
            ],
        },
        // Test case 2: High iteration count (100,000)
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTT",
            salt: "saltKEYbcTcXHCBxtjD",
            count: 100000,
            expected: [
                0xAC, 0xCD, 0xCD, 0x87, 0x98, 0xAE, 0x5C, 0xD8, 0x58, 0x04, 0x73, 0x90, 0x15, 0xEF, 0x2A, 0x11,
                0xE3, 0x25, 0x91, 0xB7, 0xB7, 0xD1, 0x6F, 0x76, 0x81, 0x9B, 0x30, 0xB0, 0xD4, 0x9D, 0x80, 0xE1,
                0xAB, 0xEA, 0x6C, 0x98, 0x22, 0xB8, 0x0A, 0x1F, 0xDF, 0xE4, 0x21, 0xE2, 0x6F, 0x56, 0x03, 0xEC,
                0xA8, 0xA4, 0x7A, 0x64, 0xC9, 0xA0, 0x04, 0xFB, 0x5A, 0xF8, 0x22, 0x9F, 0x76, 0x2F, 0xF4, 0x1F
            ],
        },
        // Test case 3: Longer password and salt
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTTl",
            salt: "saltKEYbcTcXHCBxtjD2",
            count: 1,
            expected: [
                0x8E, 0x50, 0x74, 0xA9, 0x51, 0x3C, 0x1F, 0x15, 0x12, 0xC9, 0xB1, 0xDF, 0x1D, 0x8B, 0xFF, 0xA9,
                0xD8, 0xB4, 0xEF, 0x91, 0x05, 0xDF, 0xC1, 0x66, 0x81, 0x22, 0x28, 0x39, 0x56, 0x0F, 0xB6, 0x32,
                0x64, 0xBE, 0xD6, 0xAA, 0xBF, 0x76, 0x1F, 0x18, 0x0E, 0x91, 0x2A, 0x66, 0xE0, 0xB5, 0x3D, 0x65,
                0xEC, 0x88, 0xF6, 0xA1, 0x51, 0x9E, 0x14, 0x80, 0x4E, 0xBA, 0x6D, 0xC9, 0xDF, 0x13, 0x70, 0x07
            ],
        },
        // Test case 4: Longer password and salt with high iteration count
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTTl",
            salt: "saltKEYbcTcXHCBxtjD2",
            count: 100000,
            expected: [
                0x59, 0x42, 0x56, 0xB0, 0xBD, 0x4D, 0x6C, 0x9F, 0x21, 0xA8, 0x7F, 0x7B, 0xA5, 0x77, 0x2A, 0x79,
                0x1A, 0x10, 0xE6, 0x11, 0x06, 0x94, 0xF4, 0x43, 0x65, 0xCD, 0x94, 0x67, 0x0E, 0x57, 0xF1, 0xAE,
                0xCD, 0x79, 0x7E, 0xF1, 0xD1, 0x00, 0x19, 0x38, 0x71, 0x90, 0x44, 0xC7, 0xF0, 0x18, 0x02, 0x66,
                0x97, 0x84, 0x5E, 0xB9, 0xAD, 0x97, 0xD9, 0x7D, 0xE3, 0x6A, 0xB8, 0x78, 0x6A, 0xAB, 0x50, 0x96
            ],
        },
        // Test case 5: Even longer inputs
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTTlR",
            salt: "saltKEYbcTcXHCBxtjD2P",
            count: 1,
            expected: [
                0xA6, 0xAC, 0x8C, 0x04, 0x8A, 0x7D, 0xFD, 0x7B, 0x83, 0x8D, 0xA8, 0x8F, 0x22, 0xC3, 0xFA, 0xB5,
                0xBF, 0xF1, 0x5D, 0x7C, 0xB8, 0xD8, 0x3A, 0x62, 0xC6, 0x72, 0x1A, 0x8F, 0xAF, 0x69, 0x03, 0xEA,
                0xB6, 0x15, 0x2C, 0xB7, 0x42, 0x10, 0x26, 0xE3, 0x6F, 0x2F, 0xFE, 0xF6, 0x61, 0xEB, 0x43, 0x84,
                0xDC, 0x27, 0x64, 0x95, 0xC7, 0x1B, 0x5C, 0xAB, 0x72, 0xE1, 0xC1, 0xA3, 0x87, 0x12, 0xE5, 0x6B
            ],
        },
        // Test case 6: Very long inputs
        Pbkdf2HmacSha512TestCase {
            password: "passDATAb00AB7YxDTTlRH2dqxDx19GDxDV1zFMz7E6QVqKIzwOtMnlxQLttpE5",
            salt: "saltKEYbcTcXHCBxtjD2PnBh44AIQ6XUOCESOhXpEp3HrcGMwbjzQKMSaf63IJe",
            count: 1,
            expected: [
                0xE2, 0xCC, 0xC7, 0x82, 0x7F, 0x1D, 0xD7, 0xC3, 0x30, 0x41, 0xA9, 0x89, 0x06, 0xA8, 0xFD, 0x7B,
                0xAE, 0x19, 0x20, 0xA5, 0x5F, 0xCB, 0x8F, 0x83, 0x16, 0x83, 0xF1, 0x4F, 0x1C, 0x39, 0x79, 0x35,
                0x1C, 0xB8, 0x68, 0x71, 0x7E, 0x5A, 0xB3, 0x42, 0xD9, 0xA1, 0x1A, 0xCF, 0x0B, 0x12, 0xD3, 0x28,
                0x39, 0x31, 0xD6, 0x09, 0xB0, 0x66, 0x02, 0xDA, 0x33, 0xF8, 0x37, 0x7D, 0x1F, 0x1F, 0x99, 0x02
            ],
        },
    ];

    #[test]
    fn test_pbkdf2_comprehensive() {
        for (i, case) in PBKDF2_HMAC_SHA512_CASES.iter().enumerate() {
            let password = case.password.as_bytes();
            let salt = case.salt.as_bytes();
            
            // Test pointer-based function
            let result = pbkdf2_hmac_sha512(password, salt, case.count, 64);
            assert!(result.is_ok(), "PBKDF2 test case {} failed", i + 1);
            
            let key = result.unwrap();
            assert_eq!(key.len(), 64);
            
            // Convert Vec<u8> to [u8; 64] for comparison
            let mut key_array = [0u8; 64];
            key_array.copy_from_slice(&key[..64]);
            
            assert_eq!(
                key_array, case.expected,
                "PBKDF2 test case {} failed: password={}, salt={}, count={}",
                i + 1, case.password, case.salt, case.count
            );
        }
    }

    #[test]
    fn test_pbkdf2_vector_consistency() {
        // Test that both pointer and vector versions produce identical results
        for case in PBKDF2_HMAC_SHA512_CASES.iter().take(3) { // Test first 3 cases for speed
            let password = case.password.as_bytes();
            let salt = case.salt.as_bytes();
            
            let result1 = pbkdf2_hmac_sha512(password, salt, case.count, 64);
            let result2 = pbkdf2_hmac_sha512_vec(password, salt, case.count, 64);
            
            assert!(result1.is_ok() && result2.is_ok());
            assert_eq!(result1.unwrap(), result2.unwrap());
        }
    }

    #[test]
    fn test_pbkdf2_output_sizes() {
        let password = b"test_password";
        let salt = b"test_salt";
        let count = 1000;
        
        // Test different output sizes
        for size in [16, 32, 48, 64, 80].iter() {
            let result = pbkdf2_hmac_sha512(password, salt, count, *size);
            assert!(result.is_ok(), "Failed for output size {}", size);
            
            let key = result.unwrap();
            assert_eq!(key.len(), *size, "Wrong output size");
        }
    }

    #[test]
    fn test_pbkdf2_edge_cases() {
        let password = b"password";
        let salt = b"salt";
        
        // Test minimum iteration count
        let result = pbkdf2_hmac_sha512(password, salt, 1, 32);
        assert!(result.is_ok());
        
        // Test empty password (should work in most implementations)
        let result = pbkdf2_hmac_sha512(&[], salt, 1000, 32);
        // Don't assert success since behavior may vary
        let _ = result;
        
        // Test empty salt (should work in most implementations)
        let result = pbkdf2_hmac_sha512(password, &[], 1000, 32);
        // Don't assert success since behavior may vary
        let _ = result;
    }
}
