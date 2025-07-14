use crate::{ffi, Network};

/// Configuration utilities for the Equity library
pub struct Configuration;

impl Configuration {
    /// Get the current network ID
    pub fn network_id() -> u32 {
        ffi::configurationGetNetworkId()
    }

    /// Get the current network
    pub fn network() -> Network {
        Network::from(Self::network_id())
    }

    /// Get the address version byte for the current network
    pub fn address_version() -> u8 {
        ffi::configurationGetAddressVersion() as u8
    }

    /// Get the private key version byte for the current network
    pub fn private_key_version() -> u8 {
        ffi::configurationGetPrivateKeyVersion() as u8
    }
}

// Standalone function exports for convenience
pub fn get_network_id() -> u32 {
    Configuration::network_id()
}

pub fn get_network() -> Network {
    Configuration::network()
}

pub fn get_address_version() -> u8 {
    Configuration::address_version()
}

pub fn get_private_key_version() -> u8 {
    Configuration::private_key_version()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_configuration_values() {
        let network_id = Configuration::network_id();
        let network = Configuration::network();
        let address_version = Configuration::address_version();
        let private_key_version = Configuration::private_key_version();
        
        println!("Network ID: {}", network_id);
        println!("Network: {:?}", network);
        println!("Address Version: 0x{:02X}", address_version);
        println!("Private Key Version: 0x{:02X}", private_key_version);
        
        // Basic sanity checks
        assert!(network_id <= 2); // Should be 0, 1, or 2 for mainnet, testnet, regtest
    }

    #[test]
    fn test_network_conversion() {
        let network = Configuration::network();
        let network_id = Configuration::network_id();
        
        // Test round-trip conversion
        assert_eq!(u32::from(network), network_id);
        assert_eq!(Network::from(network_id), network);
    }
}
