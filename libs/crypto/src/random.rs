//! Cryptographically-secure random byte generation.
//!
//! Backed by [`getrandom`], which uses the OS RNG on every supported target
//! (BCryptGenRandom on Windows, getrandom(2)/dev/urandom on Linux, /dev/urandom on macOS).

use getrandom::getrandom;

/// Returns `size` cryptographically-secure random bytes.
///
/// # Panics
///
/// Panics if the OS RNG is unavailable.
pub fn get_bytes(size: usize) -> Vec<u8> {
    let mut buffer = vec![0u8; size];
    getrandom(&mut buffer).expect("OS RNG (getrandom) failed");
    buffer
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn requested_length_returned() {
        for size in [1usize, 16, 32, 64, 128] {
            assert_eq!(get_bytes(size).len(), size);
        }
    }

    #[test]
    fn two_draws_differ() {
        let a = get_bytes(32);
        let b = get_bytes(32);
        assert_ne!(a, b);
    }

    #[test]
    fn output_is_not_all_zero() {
        let bytes = get_bytes(32);
        assert!(bytes.iter().any(|&x| x != 0));
    }
}
