//! RIPEMD-160 hashing functions

use crate::ffi;

// Constants
pub const RIPEMD160_HASH_SIZE: usize = 20; // 160 bits / 8 = 20 bytes
pub type Ripemd160Hash = [u8; RIPEMD160_HASH_SIZE];

// Safe Rust wrapper using raw pointers
pub fn ripemd160(input: &[u8]) -> Ripemd160Hash {
    let mut output = [0u8; RIPEMD160_HASH_SIZE];
    unsafe {
        ffi::ripemd160(input.as_ptr(), input.len(), output.as_mut_ptr());
    }
    output
}

// NEW: Vector-based convenience function
pub fn ripemd160_vec(input: &[u8]) -> Vec<u8> {
    let input_vec = input.to_vec();
    let mut output = Vec::new();
    ffi::ripemd160Vector(&input_vec, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    // Test cases ported from TestRipemd.cpp
    struct Ripemd160TestCase {
        input: &'static str,
        expected: [u8; RIPEMD160_HASH_SIZE],
    }

    const RIPEMD160_CASES: &[Ripemd160TestCase] = &[
        Ripemd160TestCase {
            input: "",
            expected: [0x9c, 0x11, 0x85, 0xa5, 0xc5, 0xe9, 0xfc, 0x54, 0x61, 0x28, 0x08, 0x97, 0x7e, 0xe8, 0xf5, 0x48, 0xb2, 0x25, 0x8d, 0x31],
        },
        Ripemd160TestCase {
            input: "a",
            expected: [0x0b, 0xdc, 0x9d, 0x2d, 0x25, 0x6b, 0x3e, 0xe9, 0xda, 0xae, 0x34, 0x7b, 0xe6, 0xf4, 0xdc, 0x83, 0x5a, 0x46, 0x7f, 0xfe],
        },
        Ripemd160TestCase {
            input: "abc",
            expected: [0x8e, 0xb2, 0x08, 0xf7, 0xe0, 0x5d, 0x98, 0x7a, 0x9b, 0x04, 0x4a, 0x8e, 0x98, 0xc6, 0xb0, 0x87, 0xf1, 0x5a, 0x0b, 0xfc],
        },
        Ripemd160TestCase {
            input: "message digest",
            expected: [0x5d, 0x06, 0x89, 0xef, 0x49, 0xd2, 0xfa, 0xe5, 0x72, 0xb8, 0x81, 0xb1, 0x23, 0xa8, 0x5f, 0xfa, 0x21, 0x59, 0x5f, 0x36],
        },
        Ripemd160TestCase {
            input: "abcdefghijklmnopqrstuvwxyz",
            expected: [0xf7, 0x1c, 0x27, 0x10, 0x9c, 0x69, 0x2c, 0x1b, 0x56, 0xbb, 0xdc, 0xeb, 0x5b, 0x9d, 0x28, 0x65, 0xb3, 0x70, 0x8d, 0xbc],
        },
        Ripemd160TestCase {
            input: "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
            expected: [0x12, 0xa0, 0x53, 0x38, 0x4a, 0x9c, 0x0c, 0x88, 0xe4, 0x05, 0xa0, 0x6c, 0x27, 0xdc, 0xf4, 0x9a, 0xda, 0x62, 0xeb, 0x2b],
        },
        Ripemd160TestCase {
            input: "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
            expected: [0xb0, 0xe2, 0x0b, 0x6e, 0x31, 0x16, 0x64, 0x02, 0x86, 0xed, 0x3a, 0x87, 0xa5, 0x71, 0x30, 0x79, 0xb2, 0x1f, 0x51, 0x89],
        },
        Ripemd160TestCase {
            input: "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
            expected: [0x9b, 0x75, 0x2e, 0x45, 0x57, 0x3d, 0x4b, 0x39, 0xf4, 0xdb, 0xd3, 0x32, 0x3c, 0xab, 0x82, 0xbf, 0x63, 0x32, 0x6b, 0xfb],
        },
    ];

    #[test]
    fn test_ripemd160_ptr() {
        for case in RIPEMD160_CASES {
            let result = ripemd160(case.input.as_bytes());
            assert_eq!(result, case.expected, "RIPEMD160 test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_ripemd160_vector() {
        for case in RIPEMD160_CASES {
            let result = ripemd160_vec(case.input.as_bytes());
            assert_eq!(result, case.expected.to_vec(), "RIPEMD160 vector test failed for input: '{}'", case.input);
        }
    }

    #[test]
    fn test_ripemd160_array() {
        let input = [b'a', b'b', b'c'];
        let expected = [0x8e, 0xb2, 0x08, 0xf7, 0xe0, 0x5d, 0x98, 0x7a, 0x9b, 0x04, 0x4a, 0x8e, 0x98, 0xc6, 0xb0, 0x87, 0xf1, 0x5a, 0x0b, 0xfc];
        let result = ripemd160(&input);
        assert_eq!(result, expected);
    }
}
