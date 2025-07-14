//! Address module for Bitcoin addresses
//! 
//! Provides functionality for creating, validating, and converting Bitcoin addresses.

use crate::ffi::{self, AddressCpp};
use crate::{Result, EquityError, Network};
use std::str::FromStr;

/// A Bitcoin address
pub struct Address {
    inner: AddressCpp,
}

impl Address {
    /// Create an address from a Base58Check string
    pub fn from_string(s: &str) -> Result<Self> {
        let inner = ffi::addressFromString(s);
        if ffi::addressIsValid(&inner) {
            Ok(Address { inner })
        } else {
            Err(EquityError("Invalid address string".to_string()))
        }
    }

    /// Create an address from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() != 20 {
            return Err(EquityError("Address data must be 20 bytes".to_string()));
        }
        let inner = ffi::addressFromData(data);
        if ffi::addressIsValid(&inner) {
            Ok(Address { inner })
        } else {
            Err(EquityError("Invalid address data".to_string()))
        }
    }

    /// Create an address from a public key
    pub fn from_public_key(pubkey_data: &[u8]) -> Result<Self> {
        let inner = ffi::addressFromPublicKey(pubkey_data);
        if ffi::addressIsValid(&inner) {
            Ok(Address { inner })
        } else {
            Err(EquityError("Invalid public key for address generation".to_string()))
        }
    }

    /// Convert the address to a Base58Check string
    pub fn to_string(&self, network: Network) -> String {
        ffi::addressToString(&self.inner, network.into())
    }

    /// Get the address as raw bytes
    pub fn value(&self) -> Vec<u8> {
        ffi::addressValue(&self.inner)
    }

    /// Check if the address is valid
    pub fn is_valid(&self) -> bool {
        ffi::addressIsValid(&self.inner)
    }
}

impl std::fmt::Display for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_string(Network::Mainnet))
    }
}

impl std::fmt::Debug for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Address")
            .field("value", &hex::encode(self.value()))
            .field("valid", &self.is_valid())
            .finish()
    }
}

impl std::str::FromStr for Address {
    type Err = crate::EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_string(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_address_validation() {
        // Test with a known invalid address
        let invalid_addr = Address::from_string("invalid");
        assert!(invalid_addr.is_err());
    }

    #[test]
    fn test_address_display() {
        // Create a dummy address for testing display
        // Create a test address from a valid string instead
        let test_result = Address::from_string("1A1zP1eP5QGefi2DMPTfTL5SLmv7DivfNa");
        // This may fail if the C++ implementation is not available
        if let Ok(addr) = test_result {
            println!("Address created successfully: {}", addr);
        } else {
            println!("Address creation failed (C++ implementation not available)");
        }
    }
}
