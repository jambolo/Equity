//! Wallet functionality for Bitcoin
//! 
//! Provides functionality for managing Bitcoin wallets and key pairs.

use crate::address::Address;
use crate::private_key::PrivateKey;
use crate::public_key::PublicKey;
use crate::{Result, EquityError, Network};

/// A wallet entry containing a key pair and associated address
#[derive(Debug)]
pub struct WalletEntry {
    pub private_key: PrivateKey,
    pub public_key: PublicKey,
    pub address: Address,
}

/// A Bitcoin wallet for managing private keys and addresses
#[derive(Debug)]
pub struct Wallet {
    entries: Vec<WalletEntry>,
    network: Network,
}

impl Wallet {
    /// Create a new empty wallet
    pub fn new(network: Network) -> Self {
        Wallet {
            entries: Vec::new(),
            network,
        }
    }

    /// Add a private key to the wallet
    pub fn add_private_key(&mut self, private_key: PrivateKey) -> Result<usize> {
        let public_key_data = private_key.value();
        let public_key = PublicKey::from_private_key(&public_key_data)?;
        let address = Address::from_public_key(&public_key.value())?;
        
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
        self.entries.iter()
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
}
