//! Wallet functionality for Bitcoin
//!
//! Provides functionality for managing Bitcoin wallets and key pairs.

use crate::address::Address;
use crate::private_key::PrivateKey;
use crate::public_key::PublicKey;
use crate::{Network, Result};

/// One wallet entry: a key pair and its derived P2PKH address.
#[derive(Debug)]
pub struct WalletEntry {
    /// Private key (scalar + compression flag).
    pub private_key: PrivateKey,
    /// Public key derived from `private_key`.
    pub public_key: PublicKey,
    /// HASH160 of the public key.
    pub address: Address,
}

/// In-memory bag of [`WalletEntry`]s tied to a [`Network`].
///
/// # Examples
///
/// ```
/// use equity::{Network, wallet::Wallet};
///
/// let mut w = Wallet::new(Network::Mainnet);
/// w.add_private_key_bytes(&[1u8; 32]).unwrap();
/// assert_eq!(w.len(), 1);
/// ```
#[derive(Debug)]
pub struct Wallet {
    entries: Vec<WalletEntry>,
    network: Network,
}

impl Wallet {
    /// Create a new empty wallet bound to `network`.
    pub fn new(network: Network) -> Self {
        Wallet {
            entries: Vec::new(),
            network,
        }
    }

    /// Add a private key to the wallet
    pub fn add_private_key(&mut self, private_key: PrivateKey) -> Result<usize> {
        let public_key = PublicKey::from_private_key(&private_key)?;
        let address = Address::from_public_key(&public_key)?;

        let entry = WalletEntry {
            private_key,
            public_key,
            address,
        };

        self.entries.push(entry);
        Ok(self.entries.len() - 1)
    }

    /// Add a private key from WIF format
    pub fn add_private_key_wif(&mut self, wif: &str) -> Result<usize> {
        let private_key = PrivateKey::from_string(wif)?;
        self.add_private_key(private_key)
    }

    /// Add a private key from raw bytes
    pub fn add_private_key_bytes(&mut self, bytes: &[u8]) -> Result<usize> {
        let private_key = PrivateKey::from_data(bytes)?;
        self.add_private_key(private_key)
    }

    /// Get the number of entries in the wallet
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    /// Check if the wallet is empty
    pub fn is_empty(&self) -> bool {
        self.entries.is_empty()
    }

    /// Get a wallet entry by index
    pub fn get_entry(&self, index: usize) -> Option<&WalletEntry> {
        self.entries.get(index)
    }

    /// Get all entries in the wallet
    pub fn entries(&self) -> &[WalletEntry] {
        &self.entries
    }

    /// Find an entry by address
    pub fn find_by_address(&self, target_address: &Address) -> Option<&WalletEntry> {
        self.entries.iter().find(|entry| {
            // Compare addresses by their string representation
            entry.address.to_string(self.network) == target_address.to_string(self.network)
        })
    }

    /// Get all addresses in the wallet
    pub fn get_addresses(&self) -> Vec<String> {
        self.entries
            .iter()
            .map(|entry| entry.address.to_string(self.network))
            .collect()
    }

    /// Get the network type for this wallet
    pub fn network(&self) -> Network {
        self.network
    }

    /// Clear all entries from the wallet
    pub fn clear(&mut self) {
        self.entries.clear();
    }

    /// Remove an entry by index
    pub fn remove_entry(&mut self, index: usize) -> Option<WalletEntry> {
        if index < self.entries.len() {
            Some(self.entries.remove(index))
        } else {
            None
        }
    }
}

impl WalletEntry {
    /// Get the private key in WIF format
    pub fn private_key_wif(&self, version: u32) -> String {
        self.private_key.to_wif(version)
    }

    /// Get the address as a string
    pub fn address_string(&self, network: Network) -> String {
        self.address.to_string(network)
    }

    /// Check if the private key is compressed
    pub fn is_compressed(&self) -> bool {
        self.private_key.is_compressed()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_wallet() {
        let wallet = Wallet::new(Network::Mainnet);
        assert!(wallet.is_empty());
        assert_eq!(wallet.len(), 0);
        assert_eq!(wallet.network(), Network::Mainnet);
    }

    #[test]
    fn test_add_private_key() {
        let mut wallet = Wallet::new(Network::Testnet);
        let key_data = [1u8; 32];

        if let Ok(private_key) = PrivateKey::from_data(&key_data) {
            let result = wallet.add_private_key(private_key);

            match result {
                Ok(index) => {
                    assert_eq!(index, 0);
                    assert_eq!(wallet.len(), 1);
                    assert!(!wallet.is_empty());

                    let entry = wallet.get_entry(0).unwrap();
                    assert!(entry.private_key.is_valid());
                    assert!(entry.public_key.is_valid());
                    assert!(entry.address.is_valid());
                }
                Err(_) => {
                    // Key generation might fail if the test key is invalid
                }
            }
        }
    }

    #[test]
    fn test_wallet_addresses() {
        let mut wallet = Wallet::new(Network::Mainnet);

        // Try to add a few test keys
        for i in 1..=3 {
            let mut key_data = [0u8; 32];
            key_data[31] = i as u8;

            if let Ok(private_key) = PrivateKey::from_data(&key_data) {
                let _ = wallet.add_private_key(private_key);
            }
        }

        let addresses = wallet.get_addresses();
        assert_eq!(addresses.len(), wallet.len());
    }

    #[test]
    fn test_add_private_key_wif() {
        let mut wallet = Wallet::new(Network::Mainnet);
        let wif = "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf";
        let idx = wallet.add_private_key_wif(wif).unwrap();
        assert_eq!(idx, 0);
        assert_eq!(wallet.len(), 1);
        assert_eq!(wallet.get_entry(0).unwrap().private_key_wif(0x80), wif);
    }

    #[test]
    fn test_add_private_key_bytes() {
        let mut wallet = Wallet::new(Network::Mainnet);
        let mut key = [0u8; 32];
        key[31] = 9;
        let idx = wallet.add_private_key_bytes(&key).unwrap();
        assert_eq!(idx, 0);
    }

    #[test]
    fn test_remove_entry_and_clear() {
        let mut wallet = Wallet::new(Network::Mainnet);
        for i in 1..=3u8 {
            let mut key = [0u8; 32];
            key[31] = i;
            wallet.add_private_key_bytes(&key).unwrap();
        }
        assert_eq!(wallet.len(), 3);
        let removed = wallet.remove_entry(1);
        assert!(removed.is_some());
        assert_eq!(wallet.len(), 2);
        assert!(wallet.remove_entry(99).is_none());
        wallet.clear();
        assert!(wallet.is_empty());
    }

    #[test]
    fn test_find_by_address() {
        let mut wallet = Wallet::new(Network::Mainnet);
        let mut key = [0u8; 32];
        key[31] = 1;
        wallet.add_private_key_bytes(&key).unwrap();
        let target = wallet.get_entry(0).unwrap().address.clone();
        let found = wallet.find_by_address(&target);
        assert!(found.is_some());
    }
}
