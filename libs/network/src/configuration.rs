//! Network configuration (magic number and TCP port).

use crate::header::MAGIC_MAIN;

#[derive(Debug, Clone, Copy)]
pub struct Configuration {
    pub network: u32,
    pub port: u16,
}

pub const DEFAULT: Configuration = Configuration {
    network: MAGIC_MAIN,
    port: 8333,
};

impl Configuration {
    pub fn default_mainnet() -> Self {
        DEFAULT
    }
}

pub fn get_network() -> u32 {
    DEFAULT.network
}

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
