//! Network configuration utilities

use crate::ffi;

/// Network configuration settings
pub struct Configuration;

impl Configuration {
    /// Get the network magic number
    pub fn network() -> u32 {
        ffi::networkConfigurationGetNetwork()
    }
    
    /// Get the network port
    pub fn port() -> i32 {
        ffi::networkConfigurationGetPort()
    }
}

// Convenience functions
pub fn get_network() -> u32 {
    Configuration::network()
}

pub fn get_port() -> i32 {
    Configuration::port()
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_configuration_access() {
        let network = get_network();
        let port = get_port();
        
        // Just test that we can call these functions
        println!("Network: {}, Port: {}", network, port);
    }
}
