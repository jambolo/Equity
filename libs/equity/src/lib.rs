//! Equity Rust Library
//! 
//! This library provides Rust bindings for the Equity C++ cryptocurrency library
//! using the cxx crate for safe FFI.

use cxx::{UniquePtr};

#[cxx::bridge]
mod ffi {
    #[namespace = "Equity"]
    struct AddressCpp {
        valid: bool,
    }

    #[namespace = "Equity"]
    struct PrivateKeyCpp {
        valid: bool,
        compressed: bool,
    }

    #[namespace = "Equity"]
    struct PublicKeyCpp {
        valid: bool,
        compressed: bool,
    }

    #[namespace = "Equity"]
    struct TransactionInputCpp {
        output_index: u32,
        sequence: u32,
    }

    #[namespace = "Equity"]
    struct TransactionOutputCpp {
        value: u64,
    }

    #[namespace = "Equity"]
    struct TransactionCpp {
        version: u32,
        lock_time: u32,
        valid: bool,
    }

    #[namespace = "Equity"]
    struct BlockHeaderCpp {
        version: i32,
        timestamp: u32,
        target: u32,
        nonce: u32,
    }

    // Helper struct for merkle tree proof since Vec<Vec<u8>> is not supported
    #[derive(Clone, Debug)]
    struct MerkleProofCpp {
        proof_hashes: Vec<u8>, // Serialized hash data
        proof_count: usize,    // Number of hashes
    }

    unsafe extern "C++" {
        include!("equity_wrapper.h");

        type BlockCpp;
        type TransactionCpp;

        // Configuration functions
        fn configurationGetNetworkId() -> u32;
        fn configurationGetAddressVersion() -> u32;
        fn configurationGetPrivateKeyVersion() -> u32;

        // Address functions
        fn addressFromString(s: &str) -> AddressCpp;
        fn addressFromData(data: &[u8]) -> AddressCpp;
        fn addressFromPublicKey(pubkey_data: &[u8]) -> AddressCpp;
        fn addressToString(address: &AddressCpp, network: u32) -> String;
        fn addressValue(address: &AddressCpp) -> Vec<u8>;
        fn addressIsValid(address: &AddressCpp) -> bool;

        // PrivateKey functions
        fn privateKeyFromData(data: &[u8]) -> PrivateKeyCpp;
        fn privateKeyFromString(s: &str) -> PrivateKeyCpp;
        fn privateKeyValue(pk: &PrivateKeyCpp) -> Vec<u8>;
        fn privateKeyIsValid(pk: &PrivateKeyCpp) -> bool;
        fn privateKeyIsCompressed(pk: &PrivateKeyCpp) -> bool;
        fn privateKeySetCompressed(pk: &mut PrivateKeyCpp, compressed: bool);
        fn privateKeyToWif(pk: &PrivateKeyCpp, version: u32) -> String;
        fn privateKeyToHex(pk: &PrivateKeyCpp) -> String;

        // PublicKey functions
        fn publicKeyFromData(data: &[u8]) -> PublicKeyCpp;
        fn publicKeyFromPrivateKey(pk_data: &[u8]) -> PublicKeyCpp;
        fn publicKeyValue(pubkey: &PublicKeyCpp) -> Vec<u8>;
        fn publicKeyIsValid(pubkey: &PublicKeyCpp) -> bool;
        fn publicKeyIsCompressed(pubkey: &PublicKeyCpp) -> bool;

        // Base58 functions
        fn base58Encode(input: &[u8]) -> String;
        fn base58Decode(input: &str, output: &mut Vec<u8>) -> bool;

        // Base58Check functions
        fn base58CheckEncode(input: &[u8], version: u32) -> String;
        fn base58CheckDecode(input: &str, output: &mut Vec<u8>, version: &mut u32) -> bool;

        // Transaction functions
        fn transactionFromJson(json: &str) -> UniquePtr<TransactionCpp>;
        fn transactionFromData(data: &[u8]) -> UniquePtr<TransactionCpp>;
        fn transactionToJson(tx: &TransactionCpp) -> String;
        fn transactionSerialize(tx: &TransactionCpp) -> Vec<u8>;
        fn transactionVersion(tx: &TransactionCpp) -> u32;
        fn transactionLockTime(tx: &TransactionCpp) -> u32;
        fn transactionIsValid(tx: &TransactionCpp) -> bool;
        fn transactionInputCount(tx: &TransactionCpp) -> usize;
        fn transactionOutputCount(tx: &TransactionCpp) -> usize;
        fn transactionGetInput(tx: &TransactionCpp, index: usize) -> TransactionInputCpp;
        fn transactionGetOutput(tx: &TransactionCpp, index: usize) -> TransactionOutputCpp;

        // Block functions
        fn blockFromData(data: &[u8]) -> UniquePtr<BlockCpp>;
        fn blockToJson(block: &BlockCpp) -> String;
        fn blockSerialize(block: &BlockCpp) -> Vec<u8>;
        fn blockGetHeader(block: &BlockCpp) -> BlockHeaderCpp;
        fn blockTransactionCount(block: &BlockCpp) -> usize;
        fn blockGetTransaction(block: &BlockCpp, index: usize) -> UniquePtr<TransactionCpp>;

        // Txid functions
        fn txidFromData(data: &[u8]) -> Vec<u8>;
        fn txidFromJson(json: &str) -> Vec<u8>;
        fn txidToJson(hash: &[u8]) -> String;
        fn txidSerialize(hash: &[u8]) -> Vec<u8>;

        // Mnemonic functions
        fn mnemonicGenerate(strength: u32, output: &mut String) -> bool;
        fn mnemonicValidate(mnemonic: &str) -> bool;
        fn mnemonicToSeed(mnemonic: &str, passphrase: &str, seed: &mut Vec<u8>) -> bool;

        // Script functions
        fn scriptFromData(data: &[u8]) -> Vec<u8>;
        fn scriptToHex(script: &[u8]) -> String;
        fn scriptIsValid(script: &[u8]) -> bool;

        // Target functions
        fn targetFromCompact(compact: u32) -> Vec<u8>;
        fn targetToCompact(target: &[u8]) -> u32;
        fn targetGetDifficulty(target: &[u8]) -> f64;

        // MerkleTree functions  
        fn merkleTreeCreate(hashes: &[u8], hash_count: usize) -> Vec<u8>;
        fn merkleTreeGetRoot(tree_data: &[u8]) -> Vec<u8>;
        fn merkleTreeGetProof(tree_data: &[u8], index: usize) -> MerkleProofCpp;
        fn merkleTreeVerify(hash: &[u8], index: usize, proof_data: &[u8], proof_count: usize, root: &[u8]) -> bool;
    }
}

pub mod address;
pub mod base58;
pub mod base58_check;
pub mod block;
pub mod configuration;
pub mod mnemonic;
pub mod private_key;
pub mod public_key;
pub mod script;
pub mod target;
pub mod transaction;
pub mod txid;
pub mod wallet;
pub mod merkle_tree;

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
