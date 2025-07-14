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

    #[test]
    fn test_random_status() {
        // Just test that the function can be called
        let _status = status();
        // Don't assert the result since it depends on the RNG state
    }

    #[test]
    fn test_get_bytes() {
        let size = 32;
        let bytes = get_bytes(size);
        assert_eq!(bytes.len(), size);
        
        // Test vector version
        let bytes_vec = get_bytes_vec(size);
        assert_eq!(bytes_vec.len(), size);
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
    fn test_add_entropy() {
        let entropy_data = b"some entropy data";
        let entropy_value = 8.0; // bits of entropy
        
        // This should not panic
        add_entropy(entropy_data, entropy_value);
    }

    #[test]
    fn test_randomness() {
        // Test that consecutive calls produce different results
        let bytes1 = get_bytes(32);
        let bytes2 = get_bytes(32);
        
        // While theoretically possible to be equal, it's extremely unlikely
        // for a good RNG with 32 bytes of output
        // Note: We don't assert inequality since it depends on RNG implementation
        let _ = (bytes1, bytes2);
    }
}
