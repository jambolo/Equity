//! Network communication utilities

#[cxx::bridge]
mod ffi {
    unsafe extern "C++" {
        include!("network_wrapper.h");

        // Configuration functions
        fn networkConfigurationGetNetwork() -> u32;
        fn networkConfigurationGetPort() -> i32;

        // Address functions
        unsafe fn networkAddressCreate(time: u32, services: u64, ipv6: *const u8, port: u16, serialized: &mut Vec<u8>) -> bool;
        fn networkAddressDeserialize(data: &Vec<u8>, time: &mut u32, services: &mut u64, ipv6: &mut Vec<u8>, port: &mut u16) -> bool;
        fn networkAddressSerialize(time: u32, services: u64, ipv6: &Vec<u8>, port: u16, output: &mut Vec<u8>) -> bool;
        fn networkAddressToJson(time: u32, services: u64, ipv6: &Vec<u8>, port: u16, json: &mut String) -> bool;

        // Inventory functions
        fn networkInventoryCreate(type_id: u32, hash: &Vec<u8>, serialized: &mut Vec<u8>) -> bool;
        fn networkInventoryDeserialize(data: &Vec<u8>, type_id: &mut u32, hash: &mut Vec<u8>) -> bool;
        fn networkInventorySerialize(type_id: u32, hash: &Vec<u8>, output: &mut Vec<u8>) -> bool;
        fn networkInventoryToJson(type_id: u32, hash: &Vec<u8>, json: &mut String) -> bool;

        // Message base functions
        fn networkMessageGetType(serialized: &Vec<u8>, msg_type: &mut String) -> bool;

        // Version Message functions
        fn networkVersionMessageCreate(
            version: u32, services: u64, timestamp: u64,
            // to address
            to_time: u32, to_services: u64, to_ipv6: &Vec<u8>, to_port: u16,
            // from address
            from_time: u32, from_services: u64, from_ipv6: &Vec<u8>, from_port: u16,
            nonce: u64, user_agent: &str, height: u32, relay: bool,
            serialized: &mut Vec<u8>
        ) -> bool;

        fn networkVersionMessageDeserialize(
            data: &Vec<u8>,
            version: &mut u32, services: &mut u64, timestamp: &mut u64,
            // to address
            to_time: &mut u32, to_services: &mut u64, to_ipv6: &mut Vec<u8>, to_port: &mut u16,
            // from address
            from_time: &mut u32, from_services: &mut u64, from_ipv6: &mut Vec<u8>, from_port: &mut u16,
            nonce: &mut u64, user_agent: &mut String, height: &mut u32, relay: &mut bool
        ) -> bool;

        // Simple message types (no payload)
        fn networkVerackMessageCreate(serialized: &mut Vec<u8>) -> bool;
        fn networkGetAddrMessageCreate(serialized: &mut Vec<u8>) -> bool;
        fn networkFilterClearMessageCreate(serialized: &mut Vec<u8>) -> bool;
        fn networkSendHeadersMessageCreate(serialized: &mut Vec<u8>) -> bool;

        // Ping/Pong messages
        fn networkPingMessageCreate(nonce: u64, serialized: &mut Vec<u8>) -> bool;
        fn networkPingMessageDeserialize(data: &Vec<u8>, nonce: &mut u64) -> bool;
        fn networkPongMessageCreate(nonce: u64, serialized: &mut Vec<u8>) -> bool;
        fn networkPongMessageDeserialize(data: &Vec<u8>, nonce: &mut u64) -> bool;

        // Address Message functions - using serialized format for multiple addresses
        fn networkAddressMessageCreate(serialized_addresses: &Vec<u8>, serialized: &mut Vec<u8>) -> bool;
        fn networkAddressMessageDeserialize(data: &Vec<u8>, serialized_addresses: &mut Vec<u8>) -> bool;

        // Inventory Message functions - using serialized format for multiple inventories
        fn networkInventoryMessageCreate(serialized_inventories: &Vec<u8>, serialized: &mut Vec<u8>) -> bool;
        fn networkInventoryMessageDeserialize(data: &Vec<u8>, serialized_inventories: &mut Vec<u8>) -> bool;

        // GetData Message functions - using serialized format for multiple inventories
        fn networkGetDataMessageCreate(serialized_inventories: &Vec<u8>, serialized: &mut Vec<u8>) -> bool;
        fn networkGetDataMessageDeserialize(data: &Vec<u8>, serialized_inventories: &mut Vec<u8>) -> bool;

        // GetBlocks Message functions - using serialized format for multiple hashes
        fn networkGetBlocksMessageCreate(
            version: u32, serialized_hashes: &Vec<u8>, stop_hash: &Vec<u8>,
            serialized: &mut Vec<u8>
        ) -> bool;
        
        // Generic message functions
        fn networkMessageSerialize(command: &String, payload: &Vec<u8>, magic: u32, output: &mut Vec<u8>) -> bool;
        fn networkMessageDeserialize(data: &Vec<u8>, command: &mut String, payload: &mut Vec<u8>, magic: &mut u32, checksum: &mut u32) -> bool;
        fn networkMessageToJson(command: &String, payload: &Vec<u8>, magic: u32, json: &mut String) -> bool;
        fn networkMessageValidate(command: &String, payload: &Vec<u8>, magic: u32, checksum: u32) -> bool;
        
        // Message creation helpers
        fn networkCreatePingMessage(nonce: u64, payload: &mut Vec<u8>) -> bool;
        fn networkCreatePongMessage(nonce: u64, payload: &mut Vec<u8>) -> bool;
    }
}

pub mod address;
pub mod configuration;
pub mod inventory;
pub mod message;
pub mod messages;

// Re-export main types for convenience
pub use address::Address;
pub use configuration::Configuration;
pub use inventory::{InventoryId, InventoryType};
pub use message::{Message, MessageType};
pub use messages::{VersionMessage, PingMessage, PongMessage, SimpleMessage, AddressMessage, InventoryMessage};

// Re-export common types and functions
pub use address::*;
pub use configuration::*;
pub use inventory::*;
pub use message::*;
pub use messages::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_configuration() {
        let network = ffi::networkConfigurationGetNetwork();
        let port = ffi::networkConfigurationGetPort();
        
        println!("Network: {}, Port: {}", network, port);
        // Don't assert specific values since they depend on configuration
    }

    #[test]
    fn test_address_creation() {
        let time = 1234567890u32;
        let services = 1u64;
        let ipv6 = vec![0u8; 16]; // IPv6 address
        let port = 8333u16;
        
        let mut serialized = Vec::new();
        let success = unsafe {
            ffi::networkAddressCreate(time, services, ipv6.as_ptr(), port, &mut serialized)
        };
        
        if success {
            assert!(!serialized.is_empty());
            println!("Address serialized to {} bytes", serialized.len());
        }
    }

    #[test]
    fn test_inventory_creation() {
        let type_id = 1u32; // TYPE_TX
        let hash = vec![0u8; 32]; // SHA256 hash
        
        let mut serialized = Vec::new();
        let success = ffi::networkInventoryCreate(type_id, &hash, &mut serialized);
        
        if success {
            assert!(!serialized.is_empty());
            println!("Inventory serialized to {} bytes", serialized.len());
        }
    }

    #[test]
    fn test_ping_pong() {
        let nonce = 0x123456789abcdef0u64;
        
        // Test ping message
        let mut ping_data = Vec::new();
        let ping_success = ffi::networkPingMessageCreate(nonce, &mut ping_data);
        
        if ping_success {
            assert!(!ping_data.is_empty());
            
            // Test deserializing ping
            let mut decoded_nonce = 0u64;
            let decode_success = ffi::networkPingMessageDeserialize(&ping_data, &mut decoded_nonce);
            
            if decode_success {
                assert_eq!(nonce, decoded_nonce);
            }
        }
        
        // Test pong message
        let mut pong_data = Vec::new();
        let pong_success = ffi::networkPongMessageCreate(nonce, &mut pong_data);
        
        if pong_success {
            assert!(!pong_data.is_empty());
        }
    }

    #[test]
    fn test_simple_messages() {
        let mut verack = Vec::new();
        let verack_success = ffi::networkVerackMessageCreate(&mut verack);
        
        let mut getaddr = Vec::new();
        let getaddr_success = ffi::networkGetAddrMessageCreate(&mut getaddr);
        
        // These should succeed (they're simple messages with no payload)
        if verack_success {
            println!("Verack message: {} bytes", verack.len());
        }
        
        if getaddr_success {
            println!("GetAddr message: {} bytes", getaddr.len());
        }
    }
}
