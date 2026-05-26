//! Equity — Bitcoin protocol primitives implemented in pure Rust.
//!
//! The crate previously bridged to a C++ implementation through `cxx`; that
//! surface has been removed and every module now stands on its own.

pub mod address;
pub mod base58;
pub mod base58_check;
pub mod block;
pub mod configuration;
pub mod instruction;
pub mod merkle_tree;
pub mod mnemonic;
mod mnemonic_wordlist;
pub mod private_key;
pub mod public_key;
pub mod script;
pub mod script_engine;
pub mod target;
pub mod transaction;
pub mod txid;
pub mod wallet;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Network {
    Mainnet = 0,
    Testnet = 1,
    Regtest = 2,
}

impl From<u32> for Network {
    fn from(value: u32) -> Self {
        match value {
            0 => Network::Mainnet,
            1 => Network::Testnet,
            2 => Network::Regtest,
            _ => Network::Mainnet,
        }
    }
}

impl From<Network> for u32 {
    fn from(network: Network) -> Self {
        network as u32
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EquityError(pub String);

impl std::fmt::Display for EquityError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Equity error: {}", self.0)
    }
}

impl std::error::Error for EquityError {}

pub type Result<T> = std::result::Result<T, EquityError>;

pub fn error(msg: &str) -> EquityError {
    EquityError(msg.to_string())
}
