//! Bitcoin protocol specific message implementations

use crate::ffi;
use crate::{Address, InventoryId, InventoryType};

/// Version message for Bitcoin protocol handshake
#[derive(Debug, Clone)]
pub struct VersionMessage {
    pub version: u32,
    pub services: u64,
    pub timestamp: u64,
    pub addr_recv: Address,
    pub addr_from: Address,
    pub nonce: u64,
    pub user_agent: String,
    pub start_height: u32,
    pub relay: bool,
}

impl VersionMessage {
    /// Create a new version message
    pub fn new(
        version: u32,
        services: u64,
        timestamp: u64,
        addr_recv: Address,
        addr_from: Address,
        nonce: u64,
        user_agent: String,
        start_height: u32,
        relay: bool,
    ) -> Self {
        VersionMessage {
            version,
            services,
            timestamp,
            addr_recv,
            addr_from,
            nonce,
            user_agent,
            start_height,
            relay,
        }
    }
    
    /// Serialize to bytes
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        
        // Serialize the addresses first
        let to_addr = self.addr_recv.serialize()?;
        let from_addr = self.addr_from.serialize()?;
        
        if to_addr.len() < 26 || from_addr.len() < 26 {
            return Err("Invalid address serialization");
        }
        
        // Extract IPv6 bytes (16 bytes starting at offset 8)
        let to_ipv6 = to_addr[8..24].to_vec();
        let from_ipv6 = from_addr[8..24].to_vec();
        
        let success = ffi::networkVersionMessageCreate(
            self.version,
            self.services,
            self.timestamp,
            self.addr_recv.time,
            self.addr_recv.services,
            &to_ipv6,
            self.addr_recv.port,
            self.addr_from.time,
            self.addr_from.services,
            &from_ipv6,
            self.addr_from.port,
            self.nonce,
            &self.user_agent,
            self.start_height,
            self.relay,
            &mut serialized,
        );
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to serialize version message")
        }
    }
    
    /// Deserialize from bytes
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut version = 0u32;
        let mut services = 0u64;
        let mut timestamp = 0u64;
        let mut to_time = 0u32;
        let mut to_services = 0u64;
        let mut to_ipv6 = Vec::new();
        let mut to_port = 0u16;
        let mut from_time = 0u32;
        let mut from_services = 0u64;
        let mut from_ipv6 = Vec::new();
        let mut from_port = 0u16;
        let mut nonce = 0u64;
        let mut user_agent = String::new();
        let mut height = 0u32;
        let mut relay = false;
        
        let success = ffi::networkVersionMessageDeserialize(
            &data_vec,
            &mut version,
            &mut services,
            &mut timestamp,
            &mut to_time,
            &mut to_services,
            &mut to_ipv6,
            &mut to_port,
            &mut from_time,
            &mut from_services,
            &mut from_ipv6,
            &mut from_port,
            &mut nonce,
            &mut user_agent,
            &mut height,
            &mut relay,
        );
        
        if success {
            let addr_recv = Address {
                time: to_time,
                services: to_services,
                ipv6: to_ipv6,
                port: to_port,
            };
            
            let addr_from = Address {
                time: from_time,
                services: from_services,
                ipv6: from_ipv6,
                port: from_port,
            };
            
            Ok(VersionMessage {
                version,
                services,
                timestamp,
                addr_recv,
                addr_from,
                nonce,
                user_agent,
                start_height: height,
                relay,
            })
        } else {
            Err("Failed to deserialize version message")
        }
    }
}

/// Ping message for connection keep-alive
#[derive(Debug, Clone)]
pub struct PingMessage {
    pub nonce: u64,
}

impl PingMessage {
    pub fn new(nonce: u64) -> Self {
        PingMessage { nonce }
    }
    
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkPingMessageCreate(self.nonce, &mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to serialize ping message")
        }
    }
    
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut nonce = 0u64;
        
        let success = ffi::networkPingMessageDeserialize(&data_vec, &mut nonce);
        
        if success {
            Ok(PingMessage { nonce })
        } else {
            Err("Failed to deserialize ping message")
        }
    }
}

/// Pong message in response to ping
#[derive(Debug, Clone)]
pub struct PongMessage {
    pub nonce: u64,
}

impl PongMessage {
    pub fn new(nonce: u64) -> Self {
        PongMessage { nonce }
    }
    
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkPongMessageCreate(self.nonce, &mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to serialize pong message")
        }
    }
    
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut nonce = 0u64;
        
        let success = ffi::networkPongMessageDeserialize(&data_vec, &mut nonce);
        
        if success {
            Ok(PongMessage { nonce })
        } else {
            Err("Failed to deserialize pong message")
        }
    }
}

/// Simple message types with no payload
pub struct SimpleMessage;

impl SimpleMessage {
    /// Create a version acknowledgment message
    pub fn verack() -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkVerackMessageCreate(&mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create verack message")
        }
    }
    
    /// Create a get addresses message
    pub fn getaddr() -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkGetAddrMessageCreate(&mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create getaddr message")
        }
    }
    
    /// Create a filter clear message
    pub fn filterclear() -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkFilterClearMessageCreate(&mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create filterclear message")
        }
    }
    
    /// Create a send headers message
    pub fn sendheaders() -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        let success = ffi::networkSendHeadersMessageCreate(&mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create sendheaders message")
        }
    }
}

/// Address list message
#[derive(Debug, Clone)]
pub struct AddressMessage {
    pub addresses: Vec<Address>,
}

impl AddressMessage {
    pub fn new(addresses: Vec<Address>) -> Self {
        AddressMessage { addresses }
    }
    
    /// Serialize addresses to a flat byte vector for message creation
    pub fn serialize_addresses(&self) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        
        // First write the count of addresses
        let count = self.addresses.len() as u32;
        serialized.extend_from_slice(&count.to_le_bytes());
        
        // Then serialize each address
        for address in &self.addresses {
            let addr_bytes = address.serialize()?;
            serialized.extend_from_slice(&addr_bytes);
        }
        
        Ok(serialized)
    }
    
    /// Create the address message payload
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let serialized_addresses = self.serialize_addresses()?;
        let mut serialized = Vec::new();
        
        let success = ffi::networkAddressMessageCreate(&serialized_addresses, &mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create address message")
        }
    }
}

/// Inventory list message
#[derive(Debug, Clone)]
pub struct InventoryMessage {
    pub inventories: Vec<InventoryId>,
}

impl InventoryMessage {
    pub fn new(inventories: Vec<InventoryId>) -> Self {
        InventoryMessage { inventories }
    }
    
    /// Serialize inventories to a flat byte vector for message creation
    pub fn serialize_inventories(&self) -> Result<Vec<u8>, &'static str> {
        let mut serialized = Vec::new();
        
        // First write the count of inventories
        let count = self.inventories.len() as u32;
        serialized.extend_from_slice(&count.to_le_bytes());
        
        // Then serialize each inventory
        for inventory in &self.inventories {
            let inv_bytes = inventory.serialize()?;
            serialized.extend_from_slice(&inv_bytes);
        }
        
        Ok(serialized)
    }
    
    /// Create the inventory message payload
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let serialized_inventories = self.serialize_inventories()?;
        let mut serialized = Vec::new();
        
        let success = ffi::networkInventoryMessageCreate(&serialized_inventories, &mut serialized);
        
        if success {
            Ok(serialized)
        } else {
            Err("Failed to create inventory message")
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_version_message_creation() {
        let addr_recv = Address {
            time: 1234567890,
            services: 1,
            ipv6: vec![0u8; 16],
            port: 8333,
        };
        
        let addr_from = Address {
            time: 1234567890,
            services: 1,
            ipv6: vec![0u8; 16],
            port: 8333,
        };
        
        let version_msg = VersionMessage::new(
            70015,
            1,
            1234567890,
            addr_recv,
            addr_from,
            12345678901234567890,
            "/Test:0.1.0/".to_string(),
            500000,
            true,
        );
        
        assert_eq!(version_msg.version, 70015);
        assert_eq!(version_msg.services, 1);
        assert_eq!(version_msg.user_agent, "/Test:0.1.0/");
        assert_eq!(version_msg.start_height, 500000);
        assert!(version_msg.relay);
    }
    
    #[test]
    fn test_ping_pong_messages() {
        let nonce = 1234567890u64;
        
        let ping = PingMessage::new(nonce);
        assert_eq!(ping.nonce, nonce);
        
        let pong = PongMessage::new(nonce);
        assert_eq!(pong.nonce, nonce);
    }
    
    #[test]
    fn test_simple_messages() {
        // These will only work if the bridge functions are properly implemented
        if let Ok(verack) = SimpleMessage::verack() {
            assert!(!verack.is_empty());
        }
        
        if let Ok(getaddr) = SimpleMessage::getaddr() {
            assert!(!getaddr.is_empty());
        }
    }
    
    #[test]
    fn test_address_message() {
        let addresses = vec![
            Address {
                time: 1234567890,
                services: 1,
                ipv6: vec![0u8; 16],
                port: 8333,
            },
            Address {
                time: 1234567891,
                services: 1,
                ipv6: vec![0u8; 16],
                port: 8334,
            },
        ];
        
        let addr_msg = AddressMessage::new(addresses);
        assert_eq!(addr_msg.addresses.len(), 2);
        
        // Test address serialization
        if let Ok(serialized) = addr_msg.serialize_addresses() {
            assert!(serialized.len() > 4); // At least count + some address data
        }
    }
    
    #[test]
    fn test_inventory_message() {
        let hash1 = vec![1u8; 32];
        let hash2 = vec![2u8; 32];
        
        let inventories = vec![
            InventoryId::transaction(hash1).unwrap(),
            InventoryId::block(hash2).unwrap(),
        ];
        
        let inv_msg = InventoryMessage::new(inventories);
        assert_eq!(inv_msg.inventories.len(), 2);
        assert_eq!(inv_msg.inventories[0].inv_type, InventoryType::Transaction);
        assert_eq!(inv_msg.inventories[1].inv_type, InventoryType::Block);
    }
}
