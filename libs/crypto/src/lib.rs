//! Cryptographic utilities and functions

pub mod hash;
pub mod signature;

pub use hash::*;
pub use signature::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
