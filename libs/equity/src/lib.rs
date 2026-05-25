//! Equity Rust Library

#[cxx::bridge]
mod ffi {
    struct AddressCpp {
        valid: bool,
    }

    struct PrivateKeyCpp {
        valid: bool,
        compressed: bool,
    }

    struct PublicKeyCpp {
        valid: bool,
        compressed: bool,
    }

    struct TransactionInputCpp {
        output_index: u32,
        sequence: u32,
    }

    struct TransactionOutputCpp {
        value: u64,
    }

    struct TransactionCpp {
        version: u32,
        lock_time: u32,
        valid: bool,
    }

    struct BlockHeaderCpp {
        version: i32,
        timestamp: u32,
        target: u32,
        nonce: u32,
    }

    struct BlockCpp {
        valid: bool,
    }

    #[derive(Clone, Debug)]
    struct MerkleProofCpp {
        proof_hashes: Vec<u8>,
        proof_count: usize,
    }

    unsafe extern "C++" {
        include!("equity_wrapper.h");

        // Configuration
        fn configurationGetNetworkId() -> u32;
        fn configurationGetAddressVersion() -> u32;
        fn configurationGetPrivateKeyVersion() -> u32;

        // Address
        fn addressFromString(s: &str) -> AddressCpp;
        fn addressFromData(data: &[u8]) -> AddressCpp;
        fn addressFromPublicKey(pubkey_data: &[u8]) -> AddressCpp;
        fn addressToString(address: &AddressCpp, network: u32) -> String;
        fn addressValue(address: &AddressCpp) -> Vec<u8>;
        fn addressIsValid(address: &AddressCpp) -> bool;

        // PrivateKey
        fn privateKeyFromData(data: &[u8]) -> PrivateKeyCpp;
        fn privateKeyFromString(s: &str) -> PrivateKeyCpp;
        fn privateKeyValue(pk: &PrivateKeyCpp) -> Vec<u8>;
        fn privateKeyIsValid(pk: &PrivateKeyCpp) -> bool;
        fn privateKeyIsCompressed(pk: &PrivateKeyCpp) -> bool;
        fn privateKeySetCompressed(pk: &mut PrivateKeyCpp, compressed: bool);
        fn privateKeyToWif(pk: &PrivateKeyCpp, version: u32) -> String;
        fn privateKeyToHex(pk: &PrivateKeyCpp) -> String;

        // PublicKey
        fn publicKeyFromData(data: &[u8]) -> PublicKeyCpp;
        fn publicKeyFromPrivateKey(pk_data: &[u8]) -> PublicKeyCpp;
        fn publicKeyValue(pubkey: &PublicKeyCpp) -> Vec<u8>;
        fn publicKeyIsValid(pubkey: &PublicKeyCpp) -> bool;
        fn publicKeyIsCompressed(pubkey: &PublicKeyCpp) -> bool;

        // Base58
        fn base58Encode(input: &[u8]) -> String;
        fn base58Decode(input: &str, output: &mut Vec<u8>) -> bool;

        // Base58Check
        fn base58CheckEncode(input: &[u8], version: u32) -> String;
        fn base58CheckDecode(input: &str, output: &mut Vec<u8>, version: &mut u32) -> bool;

        // Transaction
        fn transactionFromJson(json: &str) -> TransactionCpp;
        fn transactionFromData(data: &[u8]) -> TransactionCpp;
        fn transactionToJson(tx: &TransactionCpp) -> String;
        fn transactionSerialize(tx: &TransactionCpp) -> Vec<u8>;
        fn transactionVersion(tx: &TransactionCpp) -> u32;
        fn transactionLockTime(tx: &TransactionCpp) -> u32;
        fn transactionIsValid(tx: &TransactionCpp) -> bool;
        fn transactionInputCount(tx: &TransactionCpp) -> usize;
        fn transactionOutputCount(tx: &TransactionCpp) -> usize;
        fn transactionGetInput(tx: &TransactionCpp, index: usize) -> TransactionInputCpp;
        fn transactionGetOutput(tx: &TransactionCpp, index: usize) -> TransactionOutputCpp;

        // Block
        fn blockFromData(data: &[u8]) -> BlockCpp;
        fn blockToJson(block: &BlockCpp) -> String;
        fn blockSerialize(block: &BlockCpp) -> Vec<u8>;
        fn blockGetHeader(block: &BlockCpp) -> BlockHeaderCpp;
        fn blockTransactionCount(block: &BlockCpp) -> usize;
        fn blockGetTransaction(block: &BlockCpp, index: usize) -> TransactionCpp;

        // Txid
        fn txidFromData(data: &[u8]) -> Vec<u8>;
        fn txidFromJson(json: &str) -> Vec<u8>;
        fn txidToJson(hash: &[u8]) -> String;
        fn txidSerialize(hash: &[u8]) -> Vec<u8>;

        // Mnemonic
        fn mnemonicGenerate(strength: u32, output: &mut String) -> bool;
        fn mnemonicValidate(mnemonic: &str) -> bool;
        fn mnemonicToSeed(mnemonic: &str, passphrase: &str, seed: &mut Vec<u8>) -> bool;

        // Script
        fn scriptFromData(data: &[u8]) -> Vec<u8>;
        fn scriptToHex(script: &[u8]) -> String;
        fn scriptIsValid(script: &[u8]) -> bool;

        // Target
        fn targetFromCompact(compact: u32) -> Vec<u8>;
        fn targetToCompact(target: &[u8]) -> u32;
        fn targetGetDifficulty(target: &[u8]) -> f64;

        // MerkleTree (flat-byte API: hashes concatenated, each 32 bytes)
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

pub fn error(msg: &str) -> EquityError {
    EquityError(msg.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_configuration() {
        let network_id = ffi::configurationGetNetworkId();
        assert_eq!(network_id, 0);
    }
}
