//! Equity Rust Library
//! 
//! This library provides Rust bindings for the Equity C++ cryptocurrency library
//! using the cxx crate for safe FFI.

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("equity_wrapper.h");

        // Configuration functions
        fn configurationGetNetworkId() -> u32;
        fn configurationGetAddressVersion() -> u32;
        fn configurationGetPrivateKeyVersion() -> u32;

        // Mnemonic functions
        fn mnemonicGenerate(strength: u32, output: &mut String) -> bool;
        fn mnemonicValidate(mnemonic: &str) -> bool;
        fn mnemonicToSeed(mnemonic: &str, passphrase: &str, seed: &mut Vec<u8>) -> bool;
    }
}

// Add new equity wrapper modules
pub mod configuration;
pub mod mnemonic;

// Common types and utilities
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

#[derive(Debug)]
pub struct EquityError(pub String);

impl std::fmt::Display for EquityError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Equity error: {}", self.0)
    }
}

impl std::error::Error for EquityError {}

pub type Result<T> = std::result::Result<T, EquityError>;

/// Helper function to create an error
pub fn error(msg: &str) -> EquityError {
    EquityError(msg.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_configuration() {
        let network_id = ffi::configurationGetNetworkId();
        let address_version = ffi::configurationGetAddressVersion();
        let private_key_version = ffi::configurationGetPrivateKeyVersion();
        
        println!("Network ID: {}", network_id);
        println!("Address Version: {}", address_version);
        println!("Private Key Version: {}", private_key_version);
    }

    #[test] 
    fn test_mnemonic_generation() {
        let mut output = String::new();
        let success = ffi::mnemonicGenerate(128, &mut output);
        
        assert!(success);
        assert!(!output.is_empty());
        println!("Generated mnemonic: {}", output);
        
        // Validate the generated mnemonic
        let is_valid = ffi::mnemonicValidate(&output);
        assert!(is_valid);
    }
}
