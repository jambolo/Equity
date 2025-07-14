//! Random number generation functions

use crate::ffi;

// Safe Rust wrappers
pub fn status() -> bool {
    ffi::randomStatus()
}

pub fn get_bytes(size: usize) -> Vec<u8> {
    let mut buffer = vec![0u8; size];
    unsafe {
        ffi::randomGetBytes(buffer.as_mut_ptr(), size);
    }
    buffer
}

pub fn add_entropy(buffer: &[u8], entropy: f64) {
    unsafe {
        ffi::randomAddEntropy(buffer.as_ptr(), buffer.len(), entropy);
    }
}

// NEW: Vector-based convenience function
pub fn get_bytes_vec(size: usize) -> Vec<u8> {
    let mut output = Vec::new();
    ffi::randomGetBytesVector(size, &mut output);
    output
}

#[cfg(test)]
mod tests {
    use super::*;

    const SIZE: usize = 256;
    const SAFE_SIZE: usize = SIZE / 2 - 1; // Make it odd to catch alignment assumptions

    #[test]
    fn test_random_status() {
        // Test based on TestRandom.cpp - just verify function can be called
        let _status = status();
        // Don't assert the result since it depends on the RNG state
    }

    #[test]
    fn test_add_entropy() {
        // Test based on TestRandom.cpp - verify function can be called
        let entropy_data = b"some entropy data";
        add_entropy(entropy_data, 8.0);
        // Function should not panic
    }

    #[test]
    fn test_get_bytes() {
        // Test based on TestRandom.cpp
        let mut buffer1 = vec![0u8; SIZE];
        buffer1.fill(0);
        
        let random_bytes1 = get_bytes(SAFE_SIZE);
        buffer1[..SAFE_SIZE].copy_from_slice(&random_bytes1);
        
        // Ensure that too many bytes were not generated (remaining bytes should be zero)
        let zeros = vec![0u8; SIZE - SAFE_SIZE];
        assert_eq!(&buffer1[SAFE_SIZE..], &zeros[..]);
        
        let mut buffer2 = vec![0u8; SIZE];
        buffer2.fill(0);
        
        let random_bytes2 = get_bytes(SAFE_SIZE);
        buffer2[..SAFE_SIZE].copy_from_slice(&random_bytes2);
        
        // Ensure that the bytes vary between calls (extremely likely for good RNG)
        assert_ne!(&buffer1[..SAFE_SIZE], &buffer2[..SAFE_SIZE]);
        
        // Ensure that too few bytes were not generated (at least one should be non-zero)
        assert!(buffer1[SAFE_SIZE - 1] != 0 || buffer2[SAFE_SIZE - 1] != 0);
    }

    #[test]
    fn test_get_bytes_vec() {
        let size = 32;
        let bytes1 = get_bytes_vec(size);
        let bytes2 = get_bytes_vec(size);
        
        assert_eq!(bytes1.len(), size);
        assert_eq!(bytes2.len(), size);
        
        // Two calls should produce different results
        assert_ne!(bytes1, bytes2);
    }

    #[test]
    fn test_get_bytes_different_sizes() {
        for size in [1, 16, 32, 64, 128].iter() {
            let bytes = get_bytes(*size);
            assert_eq!(bytes.len(), *size);
            
            let bytes_vec = get_bytes_vec(*size);
            assert_eq!(bytes_vec.len(), *size);
        }
    }

    #[test]
    fn test_randomness() {
        // Test that consecutive calls produce different results
        let bytes1 = get_bytes(32);
        let bytes2 = get_bytes(32);
        
        // Basic sanity checks for randomness
        assert_eq!(bytes1.len(), 32);
        assert_eq!(bytes2.len(), 32);
        
        // Should not be all zeros (extremely unlikely)
        assert!(bytes1.iter().any(|&x| x != 0));
        assert!(bytes2.iter().any(|&x| x != 0));
        
        // Should not be all the same value (extremely unlikely)
        let first_byte1 = bytes1[0];
        assert!(bytes1.iter().any(|&x| x != first_byte1));
    }
}
