//! Cryptographically-secure random byte generation.
//!
//! Backed by [`getrandom`], which uses the OS RNG on every supported target
//! (BCryptGenRandom on Windows, getrandom(2)/dev/urandom on Linux, /dev/urandom on macOS).

use getrandom::getrandom;

/// Returns `size` cryptographically-secure random bytes.
///
/// # Panics
///
/// Panics if the OS RNG is unavailable. Matches the behaviour of the prior
/// `std::random_device`-based implementation, which aborted on failure.
pub fn get_bytes(size: usize) -> Vec<u8> {
    let mut buffer = vec![0u8; size];
    getrandom(&mut buffer).expect("OS RNG (getrandom) failed");
    buffer
}

/// No-op kept for API compatibility. The OS RNG handles its own seeding.
pub fn add_entropy(_buffer: &[u8], _entropy: f64) {}

/// Alias for [`get_bytes`] kept for API compatibility.
pub fn get_bytes_vec(size: usize) -> Vec<u8> {
    get_bytes(size)
}

#[cfg(test)]
mod tests {
    use super::*;

    const SIZE: usize = 256;
    const SAFE_SIZE: usize = SIZE / 2 - 1;

    #[test]
    fn test_add_entropy() {
        add_entropy(b"some entropy data", 8.0);
    }

    #[test]
    fn test_get_bytes() {
        let mut buffer1 = vec![0u8; SIZE];
        let random_bytes1 = get_bytes(SAFE_SIZE);
        buffer1[..SAFE_SIZE].copy_from_slice(&random_bytes1);

        let zeros = [0u8; SIZE - SAFE_SIZE];
        assert_eq!(&buffer1[SAFE_SIZE..], &zeros[..]);

        let mut buffer2 = vec![0u8; SIZE];
        let random_bytes2 = get_bytes(SAFE_SIZE);
        buffer2[..SAFE_SIZE].copy_from_slice(&random_bytes2);

        assert_ne!(&buffer1[..SAFE_SIZE], &buffer2[..SAFE_SIZE]);
        assert!(buffer1[SAFE_SIZE - 1] != 0 || buffer2[SAFE_SIZE - 1] != 0);
    }

    #[test]
    fn test_get_bytes_vec() {
        let size = 32;
        let bytes1 = get_bytes_vec(size);
        let bytes2 = get_bytes_vec(size);

        assert_eq!(bytes1.len(), size);
        assert_eq!(bytes2.len(), size);
        assert_ne!(bytes1, bytes2);
    }

    #[test]
    fn test_get_bytes_different_sizes() {
        for &size in &[1usize, 16, 32, 64, 128] {
            assert_eq!(get_bytes(size).len(), size);
            assert_eq!(get_bytes_vec(size).len(), size);
        }
    }

    #[test]
    fn test_randomness() {
        let bytes1 = get_bytes(32);
        let bytes2 = get_bytes(32);

        assert_eq!(bytes1.len(), 32);
        assert_eq!(bytes2.len(), 32);
        assert!(bytes1.iter().any(|&x| x != 0));
        assert!(bytes2.iter().any(|&x| x != 0));

        let first_byte1 = bytes1[0];
        assert!(bytes1.iter().any(|&x| x != first_byte1));
    }
}
