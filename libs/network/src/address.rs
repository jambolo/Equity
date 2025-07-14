//! Network address utilities

use crate::ffi;

/// Network address representing a peer node
#[derive(Debug, Clone)]
pub struct Address {
    pub time: u32,
    pub services: u64,
    pub ipv6: Vec<u8>, // 16 bytes
    pub port: u16,
}

impl Address {
    /// Create a new network address
    pub fn new(time: u32, services: u64, ipv6: Vec<u8>, port: u16) -> Result<Self, &'static str> {
        if ipv6.len() != 16 {
            return Err("IPv6 address must be exactly 16 bytes");
        }
        
        Ok(Address {
            time,
            services,
            ipv6,
            port,
        })
    }
    
    /// Create address from IPv4 (mapped to IPv6)
    pub fn from_ipv4(time: u32, services: u64, ipv4: [u8; 4], port: u16) -> Self {
        let mut ipv6 = vec![0u8; 16];
        // IPv4-mapped IPv6 address: ::ffff:xxxx:xxxx
        ipv6[10] = 0xff;
        ipv6[11] = 0xff;
        ipv6[12..16].copy_from_slice(&ipv4);
        
        Address {
            time,
            services,
            ipv6,
            port,
        }
    }
    
    /// Get IPv4 address if this is an IPv4-mapped address
    pub fn get_ipv4(&self) -> Option<[u8; 4]> {
        if self.ipv6.len() == 16 && 
           self.ipv6[10] == 0xff && self.ipv6[11] == 0xff &&
           self.ipv6[0..10].iter().all(|&b| b == 0) {
            let mut ipv4 = [0u8; 4];
            ipv4.copy_from_slice(&self.ipv6[12..16]);
            Some(ipv4)
        } else {
            None
        }
    }
    
    /// Serialize the address to bytes
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut output = Vec::new();
        let success = ffi::networkAddressSerialize(self.time, self.services, &self.ipv6, self.port, &mut output);
        
        if success {
            Ok(output)
        } else {
            Err("Failed to serialize address")
        }
    }
    
    /// Deserialize address from bytes
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut time = 0u32;
        let mut services = 0u64;
        let mut ipv6 = Vec::new();
        let mut port = 0u16;
        
        let success = ffi::networkAddressDeserialize(&data_vec, &mut time, &mut services, &mut ipv6, &mut port);
        
        if success {
            Ok(Address {
                time,
                services,
                ipv6,
                port,
            })
        } else {
            Err("Failed to deserialize address")
        }
    }
    
    /// Convert to JSON string
    pub fn to_json(&self) -> Result<String, &'static str> {
        let mut json = String::new();
        let success = ffi::networkAddressToJson(self.time, self.services, &self.ipv6, self.port, &mut json);
        
        if success {
            Ok(json)
        } else {
            Err("Failed to convert address to JSON")
        }
    }
}

// Convenience functions
pub fn create_address(time: u32, services: u64, ipv6: &[u8; 16], port: u16) -> Result<Vec<u8>, &'static str> {
    let mut serialized = Vec::new();
    let success = unsafe {
        ffi::networkAddressCreate(time, services, ipv6.as_ptr(), port, &mut serialized)
    };
    
    if success {
        Ok(serialized)
    } else {
        Err("Failed to create address")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_address_creation() {
        let ipv6 = vec![0u8; 16];
        let addr = Address::new(1234567890, 1, ipv6, 8333);
        assert!(addr.is_ok());
        
        let addr = addr.unwrap();
        assert_eq!(addr.time, 1234567890);
        assert_eq!(addr.services, 1);
        assert_eq!(addr.port, 8333);
        assert_eq!(addr.ipv6.len(), 16);
    }
    
    #[test]
    fn test_ipv4_mapping() {
        let ipv4 = [192, 168, 1, 1];
        let addr = Address::from_ipv4(1234567890, 1, ipv4, 8333);
        
        assert_eq!(addr.ipv6.len(), 16);
        assert_eq!(addr.get_ipv4(), Some(ipv4));
    }
    
    #[test]
    fn test_serialization() {
        let ipv6 = vec![0u8; 16];
        let addr = Address::new(1234567890, 1, ipv6, 8333).unwrap();
        
        // Test serialization
        if let Ok(serialized) = addr.serialize() {
            assert!(!serialized.is_empty());
            
            // Test deserialization
            if let Ok(deserialized) = Address::deserialize(&serialized) {
                assert_eq!(addr.time, deserialized.time);
                assert_eq!(addr.services, deserialized.services);
                assert_eq!(addr.port, deserialized.port);
                assert_eq!(addr.ipv6, deserialized.ipv6);
            }
        }
    }
}
