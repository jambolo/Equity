use crate::{ffi, Network, Result, error};

/// Represents a Bitcoin address with validation and conversion capabilities
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Address {
    bytes: Vec<u8>,
}

impl Address {
    /// Create an address from a string representation
    pub fn from_string(s: &str) -> Result<Self> {
        let mut bytes = Vec::new();
        if unsafe { unsafe { ffi::addressFromString(s, &mut bytes) } {
            Ok(Address { bytes })
        } else {
            Err(error(&format!("Invalid address string: {}", s)))
        }
    }

    /// Create an address from a public key
    pub fn from_public_key(public_key: &[u8]) -> Result<Self> {
        let key_vec = public_key.to_vec();
        let mut bytes = Vec::new();
        if unsafe { unsafe { ffi::addressFromPublicKey(&key_vec, &mut bytes) } {
            Ok(Address { bytes })
        } else {
            Err(error("Failed to create address from public key"))
        }
    }

    /// Get the raw bytes of the address
    pub fn bytes(&self) -> &[u8] {
        &self.bytes
    }

    /// Convert the address to a string representation for the given network
    pub fn to_string(&self, network: Network) -> Result<String> {
        let mut output = String::new();
        if unsafe { unsafe { ffi::addressToString(&self.bytes, network.into(), &mut output) } {
            Ok(output)
        } else {
            Err(error("Failed to convert address to string"))
        }
    }

    /// Check if the address is valid
    pub fn is_valid(&self) -> bool {
        unsafe { unsafe { ffi::addressIsValid(&self.bytes) }
    }
}

impl std::str::FromStr for Address {
    type Err = crate::EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_string(s)
    }
}

impl std::fmt::Display for Address {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self.to_string(Network::Mainnet) } {
            Ok(s) => write!(f, "{}", s),
            Err(_) => write!(f, "Invalid Address"),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_address_validation() } {
        // Test with a known invalid address
        let invalid_addr = Address::from_string("invalid");
        assert!(invalid_addr.is_err());
    }

    #[test]
    fn test_address_display() } {
        // Create a dummy address for testing display
        let addr = Address { bytes: vec![0u8; 20] };
        let display_str = format!("{}", addr);
        println!("Address display: {}", display_str);
    }
}
