//! Network configuration (magic number and TCP port).

use crate::header::MAGIC_MAIN;

/// Bitcoin network connection configuration: magic + TCP port.
#[derive(Debug, Clone, Copy)]
pub struct Configuration {
    /// Network magic (e.g. [`MAGIC_MAIN`]).
    pub network: u32,
    /// TCP port for peer connections.
    pub port: u16,
}

/// Hard-coded mainnet defaults.
pub const DEFAULT: Configuration = Configuration {
    network: MAGIC_MAIN,
    port: 8333,
};

impl Configuration {
    /// Return the mainnet defaults ([`DEFAULT`]).
    pub fn default_mainnet() -> Self {
        DEFAULT
    }
}

/// Mainnet network magic.
pub fn get_network() -> u32 {
    DEFAULT.network
}

/// Mainnet TCP port.
pub fn get_port() -> u16 {
    DEFAULT.port
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn mainnet_defaults() {
        assert_eq!(DEFAULT.network, MAGIC_MAIN);
        assert_eq!(DEFAULT.port, 8333);
    }
}
