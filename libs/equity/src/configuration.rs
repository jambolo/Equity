//! Configuration constants for the Equity library (mainnet only for now).

use crate::Network;

/// Compile-time mainnet defaults for the equity crate.
pub struct Configuration;

impl Configuration {
    /// Default network selector (mainnet).
    pub const NETWORK: Network = Network::Mainnet;
    /// Base58Check version byte for P2PKH mainnet addresses.
    pub const ADDRESS_VERSION: u8 = 0x00;
    /// Base58Check version byte for mainnet WIF private keys.
    pub const PRIVATE_KEY_VERSION: u8 = 0x80;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn mainnet_defaults() {
        assert_eq!(Configuration::NETWORK, Network::Mainnet);
        assert_eq!(Configuration::ADDRESS_VERSION, 0x00);
        assert_eq!(Configuration::PRIVATE_KEY_VERSION, 0x80);
    }
}
