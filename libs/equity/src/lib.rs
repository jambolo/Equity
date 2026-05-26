//! Equity — Bitcoin protocol primitives.
//!
//! This crate provides the Bitcoin-core data types and their wire/text
//! encodings: keys, addresses, scripts, transactions, blocks, the Merkle tree,
//! difficulty targets, BIP-39 mnemonics, and a minimal wallet.
//!
//! ## Module map
//!
//! - [`base58`] / [`base58_check`] — Base58 alphabets used by Bitcoin.
//! - [`private_key`] / [`public_key`] / [`address`] — secp256k1 key material and
//!   P2PKH derivation.
//! - [`script`] / [`instruction`] / [`script_engine`] — script parsing and a
//!   stack-based interpreter (without signature checks).
//! - [`transaction`] / [`txid`] — transaction structure and ids.
//! - [`block`] / [`merkle_tree`] / [`target`] — block headers, Merkle trees,
//!   difficulty.
//! - [`mnemonic`] — BIP-39 mnemonic generation, validation, and seed derivation.
//! - [`wallet`] — in-memory bag of `(PrivateKey, PublicKey, Address)` triples.
//! - [`configuration`] — mainnet defaults (version bytes, network id).
//!
//! Most fallible operations return [`Result`] with the crate-local
//! [`EquityError`].
//!
//! # Examples
//!
//! Derive a mainnet P2PKH address from a private key:
//!
//! ```
//! use equity::{Network, address::Address, private_key::PrivateKey, public_key::PublicKey};
//!
//! let mut k = [0u8; 32];
//! k[31] = 1;
//! let sk = PrivateKey::from_data(&k).unwrap();
//! let pk = PublicKey::from_private_key(&sk).unwrap();
//! let addr = Address::from_public_key(&pk).unwrap();
//! assert!(addr.to_string(Network::Mainnet).starts_with('1'));
//! ```
//!
//! Parse a transaction hex:
//!
//! ```
//! use equity::transaction::Transaction;
//!
//! // Coinbase of Bitcoin's genesis block.
//! let hex = "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000";
//! let bytes = hex::decode(hex).unwrap();
//! let tx = Transaction::from_data(&bytes).unwrap();
//! assert_eq!(tx.outputs()[0].value, 50_0000_0000);
//! ```

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

/// Bitcoin network selector.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Network {
    /// Production network.
    Mainnet = 0,
    /// Public test network.
    Testnet = 1,
    /// Local regression-test network.
    Regtest = 2,
}

impl TryFrom<u32> for Network {
    type Error = EquityError;

    fn try_from(value: u32) -> Result<Self> {
        match value {
            0 => Ok(Network::Mainnet),
            1 => Ok(Network::Testnet),
            2 => Ok(Network::Regtest),
            _ => Err(EquityError(format!("unknown network id {value}"))),
        }
    }
}

impl From<Network> for u32 {
    fn from(network: Network) -> Self {
        network as u32
    }
}

/// Error type returned by the equity crate.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct EquityError(
    /// Human-readable message.
    pub String,
);

impl std::fmt::Display for EquityError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Equity error: {}", self.0)
    }
}

impl std::error::Error for EquityError {}

impl From<&str> for EquityError {
    fn from(msg: &str) -> Self {
        EquityError(msg.to_string())
    }
}

impl From<String> for EquityError {
    fn from(msg: String) -> Self {
        EquityError(msg)
    }
}

/// Crate-local `Result` alias using [`EquityError`].
pub type Result<T> = std::result::Result<T, EquityError>;

#[cfg(test)]
mod lib_tests {
    use super::*;

    #[test]
    fn network_round_trip() {
        for n in [Network::Mainnet, Network::Testnet, Network::Regtest] {
            assert_eq!(Network::try_from(u32::from(n)).unwrap(), n);
        }
    }

    #[test]
    fn network_unknown_rejected() {
        assert!(Network::try_from(999).is_err());
    }

    #[test]
    fn equity_error_display_and_source() {
        let e: EquityError = "boom".into();
        assert_eq!(format!("{e}"), "Equity error: boom");
        let _: &dyn std::error::Error = &e;
    }
}
