//! SHA-256 hashing functions

use crate::ffi;

// Constants
pub const SHA256_HASH_SIZE: usize = 32; // 256 bits / 8 = 32 bytes
pub const CHECKSUM_SIZE: usize = 4; // 4 bytes
pub type Sha256Hash = [u8; SHA256_HASH_SIZE];
pub type Checksum = [u8; CHECKSUM_SIZE];

// Safe Rust wrappers using raw pointers (original functions)
pub fn sha256(input: &[u8]) -> Sha256Hash {
    let mut output = [0u8; SHA256_HASH_SIZE];
    unsafe {
        ffi::sha256(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

pub fn double_sha256(input: &[u8]) -> Sha256Hash {
    let mut output = [0u8; SHA256_HASH_SIZE];
    unsafe {
        ffi::doubleSha256(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

pub fn checksum(input: &[u8]) -> Checksum {
    let mut output = [0u8; CHECKSUM_SIZE];
    unsafe {
        ffi::checksum(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience functions
pub fn sha256_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::sha256Vector(&input_vec, &mut output);
    output
}

pub fn double_sha256_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::doubleSha256Vector(&input_vec, &mut output);
    output
}

pub fn checksum_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::checksumVector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    // Test cases ported from TestSha256.cpp
    struct Sha256TestCase {
        input: &'static str,
        expected: [u8; SHA256_HASH_SIZE],
    }

    const SHA256_CASES: &[Sha256TestCase] = &[
        Sha256TestCase {
            input: "",
            expected: [
                0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
                0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
            ],
        },
        Sha256TestCase {
            input: "abc",
            expected: [
                0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
            ],
        },
        Sha256TestCase {
            input: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            expected: [
                0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
            ],
        },
        Sha256TestCase {
            input: "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            expected: [
                0xcf, 0x5b, 0x16, 0xa7, 0x78, 0xaf, 0x83, 0x80, 0x03, 0x6c, 0xe5, 0x9e, 0x7b, 0x04, 0x92, 0x37,
                0x0b, 0x24, 0x9b, 0x11, 0xe8, 0xf0, 0x7a, 0x51, 0xaf, 0xac, 0x45, 0x03, 0x7a, 0xfe, 0xe9, 0xd1
            ],
        },
    ];

    const DOUBLE_SHA256_CASES: &[Sha256TestCase] = &[
        Sha256TestCase {
            input: "",
            expected: [
                0x5d, 0xf6, 0xe0, 0xe2, 0x76, 0x13, 0x59, 0xd3, 0x0a, 0x82, 0x75, 0x05, 0x8e, 0x29, 0x9f, 0xcc,
                0x03, 0x81, 0x53, 0x45, 0x45, 0xf5, 0x5c, 0xf4, 0x3e, 0x41, 0x98, 0x3f, 0x5d, 0x4c, 0x94, 0x56
            ],
        },
        Sha256TestCase {
            input: "abc",
            expected: [
                0x4f, 0x8b, 0x42, 0xc2, 0x2d, 0xd3, 0x72, 0x9b, 0x51, 0x9b, 0xa6, 0xf6, 0x8d, 0x2d, 0xa7, 0xcc,
                0x5b, 0x2d, 0x60, 0x6d, 0x05, 0xda, 0xed, 0x5a, 0xd5, 0x12, 0x8c, 0xc0, 0x3e, 0x6c, 0x63, 0x58
            ],
        },
        Sha256TestCase {
            input: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            expected: [
                0x0c, 0xff, 0xe1, 0x7f, 0x68, 0x95, 0x4d, 0xac, 0x3a, 0x84, 0xfb, 0x14, 0x58, 0xbd, 0x5e, 0xc9,
                0x92, 0x09, 0x44, 0x97, 0x49, 0xb2, 0xb3, 0x08, 0xb7, 0xcb, 0x55, 0x81, 0x2f, 0x95, 0x63, 0xaf
            ],
        },
        Sha256TestCase {
            input: "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
            expected: [
                0xac, 0xcd, 0x7b, 0xd1, 0xcb, 0x0f, 0xcb, 0xd8, 0x5c, 0xf0, 0xba, 0x5b, 0xa9, 0x69, 0x45, 0x12,
                0x77, 0x76, 0x37, 0x3a, 0x7d, 0x47, 0x89, 0x1e, 0xb4, 0x3e, 0xd6, 0xb1, 0xe2, 0xee, 0x60, 0xfe
            ],
        },
    ];

    #[test]
    fn test_sha256_ptr() {
        for case in SHA256_CASES {
            let result = sha256(case.input.as_bytes());
            assert_eq!(result, case.expected, "SHA256 test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_sha256_vector() {
        for case in SHA256_CASES {
            let result = sha256_vec(case.input.as_bytes());
            assert_eq!(result, case.expected.to_vec(), "SHA256 vector test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_sha256_array() {
        let input = [b'a', b'b', b'c'];
        let expected = [
            0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
            0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
        ];
        let result = sha256(&input);
        assert_eq!(result, expected);
    }

    #[test]
    fn test_double_sha256_ptr() {
        for case in DOUBLE_SHA256_CASES {
            let result = double_sha256(case.input.as_bytes());
            assert_eq!(result, case.expected, "Double SHA256 test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_double_sha256_vector() {
        for case in DOUBLE_SHA256_CASES {
            let result = double_sha256_vec(case.input.as_bytes());
            assert_eq!(result, case.expected.to_vec(), "Double SHA256 vector test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_checksum_ptr() {
        for case in DOUBLE_SHA256_CASES {
            let result = checksum(case.input.as_bytes());
            let expected = [case.expected[0], case.expected[1], case.expected[2], case.expected[3]];
            assert_eq!(result, expected, "Checksum test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_checksum_vector() {
        for case in DOUBLE_SHA256_CASES {
            let result = checksum_vec(case.input.as_bytes());
            let expected = vec![case.expected[0], case.expected[1], case.expected[2], case.expected[3]];
            assert_eq!(result, expected, "Checksum vector test failed for input: '{}'", case.input);
        }
    }
}
