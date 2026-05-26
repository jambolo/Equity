//! Configuration constants for the Equity library.
//!
//! Currently a single-instance singleton mirroring the original C++ struct.
//! Values are compile-time constants for mainnet — future work can move them
//! behind a runtime selector when testnet/regtest support is needed.

use crate::Network;

pub struct Configuration;

impl Configuration {
    pub const NETWORK_ID: u32 = Network::Mainnet as u32;
    pub const ADDRESS_VERSION: u8 = 0x00;
    pub const PRIVATE_KEY_VERSION: u8 = 0x80;

    pub fn network_id() -> u32 {
        Self::NETWORK_ID
    }

    pub fn network() -> Network {
        Network::from(Self::NETWORK_ID)
    }

    pub fn address_version() -> u8 {
        Self::ADDRESS_VERSION
    }

    pub fn private_key_version() -> u8 {
        Self::PRIVATE_KEY_VERSION
    }
}

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
        assert_eq!(Configuration::network_id(), 0);
        assert_eq!(Configuration::network(), Network::Mainnet);
        assert_eq!(Configuration::address_version(), 0x00);
        assert_eq!(Configuration::private_key_version(), 0x80);
    }

    #[test]
    fn test_network_conversion() {
        let network = Configuration::network();
        assert_eq!(u32::from(network), Configuration::network_id());
        assert_eq!(Network::from(Configuration::network_id()), network);
    }
}
