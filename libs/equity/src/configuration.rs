//! Configuration constants for the Equity library.
//!
//! Currently a single-instance singleton.
//! Values are compile-time constants for mainnet — future work can move them
//! behind a runtime selector when testnet/regtest support is needed.

use crate::Network;

/// Compile-time mainnet defaults for the equity crate.
pub struct Configuration;

impl Configuration {
    /// Default network selector (mainnet).
    pub const NETWORK_ID: u32 = Network::Mainnet as u32;
    /// Base58Check version byte for P2PKH mainnet addresses.
    pub const ADDRESS_VERSION: u8 = 0x00;
    /// Base58Check version byte for mainnet WIF private keys.
    pub const PRIVATE_KEY_VERSION: u8 = 0x80;

    /// Default network as a raw id.
    pub fn network_id() -> u32 {
        Self::NETWORK_ID
    }

    /// Default network as a [`Network`].
    pub fn network() -> Network {
        Network::from(Self::NETWORK_ID)
    }

    /// Default P2PKH address version byte.
    pub fn address_version() -> u8 {
        Self::ADDRESS_VERSION
    }

    /// Default WIF private-key version byte.
    pub fn private_key_version() -> u8 {
        Self::PRIVATE_KEY_VERSION
    }
}

/// Free-function alias for [`Configuration::network_id`].
pub fn get_network_id() -> u32 {
    Configuration::network_id()
}

/// Free-function alias for [`Configuration::network`].
pub fn get_network() -> Network {
    Configuration::network()
}

/// Free-function alias for [`Configuration::address_version`].
pub fn get_address_version() -> u8 {
    Configuration::address_version()
}

/// Free-function alias for [`Configuration::private_key_version`].
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
