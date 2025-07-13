//! Transaction and block validation functionality

pub mod transaction;
pub mod block;
pub mod script;

pub use transaction::*;
pub use block::*;
pub use script::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn it_works() {
        assert_eq!(2 + 2, 4);
    }
}
