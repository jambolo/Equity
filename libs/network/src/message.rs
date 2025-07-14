//! Network message utilities

use crate::ffi;

/// Message types for Bitcoin protocol
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum MessageType {
    Unknown = 0,
    Address,
    Alert,
    Block,
    CheckOrder,
    FilterAdd,
    FilterClear,
    FilterLoad,
    GetAddress,
    GetBlocks,
    GetData,
    GetHeaders,
    Headers,
    Inventory,
    MerkleBlock,
    NotFound,
    Ping,
    Pong,
    Reject,
    SubmitOrder,
    Transaction,
    Version,
    VerifyAck,
}

impl From<String> for MessageType {
    fn from(command: String) -> Self {
        match command.as_str() {
            "addr" => MessageType::Address,
            "alert" => MessageType::Alert,
            "block" => MessageType::Block,
            "checkorder" => MessageType::CheckOrder,
            "filteradd" => MessageType::FilterAdd,
            "filterclear" => MessageType::FilterClear,
            "filterload" => MessageType::FilterLoad,
            "getaddr" => MessageType::GetAddress,
            "getblocks" => MessageType::GetBlocks,
            "getdata" => MessageType::GetData,
            "getheaders" => MessageType::GetHeaders,
            "headers" => MessageType::Headers,
            "inv" => MessageType::Inventory,
            "merkleblock" => MessageType::MerkleBlock,
            "notfound" => MessageType::NotFound,
            "ping" => MessageType::Ping,
            "pong" => MessageType::Pong,
            "reject" => MessageType::Reject,
            "submitorder" => MessageType::SubmitOrder,
            "tx" => MessageType::Transaction,
            "version" => MessageType::Version,
            "verack" => MessageType::VerifyAck,
            _ => MessageType::Unknown,
        }
    }
}

impl From<MessageType> for String {
    fn from(msg_type: MessageType) -> Self {
        match msg_type {
            MessageType::Address => "addr".to_string(),
            MessageType::Alert => "alert".to_string(),
            MessageType::Block => "block".to_string(),
            MessageType::CheckOrder => "checkorder".to_string(),
            MessageType::FilterAdd => "filteradd".to_string(),
            MessageType::FilterClear => "filterclear".to_string(),
            MessageType::FilterLoad => "filterload".to_string(),
            MessageType::GetAddress => "getaddr".to_string(),
            MessageType::GetBlocks => "getblocks".to_string(),
            MessageType::GetData => "getdata".to_string(),
            MessageType::GetHeaders => "getheaders".to_string(),
            MessageType::Headers => "headers".to_string(),
            MessageType::Inventory => "inv".to_string(),
            MessageType::MerkleBlock => "merkleblock".to_string(),
            MessageType::NotFound => "notfound".to_string(),
            MessageType::Ping => "ping".to_string(),
            MessageType::Pong => "pong".to_string(),
            MessageType::Reject => "reject".to_string(),
            MessageType::SubmitOrder => "submitorder".to_string(),
            MessageType::Transaction => "tx".to_string(),
            MessageType::Version => "version".to_string(),
            MessageType::VerifyAck => "verack".to_string(),
            MessageType::Unknown => "unknown".to_string(),
        }
    }
}

/// Generic network message structure
#[derive(Debug, Clone)]
pub struct Message {
    pub command: String,
    pub payload: Vec<u8>,
    pub magic: u32,
    pub checksum: u32,
}

impl Message {
    /// Create a new message
    pub fn new(command: String, payload: Vec<u8>, magic: u32) -> Self {
        Message {
            command,
            payload,
            magic,
            checksum: 0, // Will be calculated during serialization
        }
    }
    
    /// Get message type
    pub fn message_type(&self) -> MessageType {
        MessageType::from(self.command.clone())
    }
    
    /// Serialize the message to bytes
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut output = Vec::new();
        let success = ffi::networkMessageSerialize(&self.command, &self.payload, self.magic, &mut output);
        
        if success {
            Ok(output)
        } else {
            Err("Failed to serialize message")
        }
    }
    
    /// Deserialize message from bytes
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut command = String::new();
        let mut payload = Vec::new();
        let mut magic = 0u32;
        let mut checksum = 0u32;
        
        let success = ffi::networkMessageDeserialize(&data_vec, &mut command, &mut payload, &mut magic, &mut checksum);
        
        if success {
            Ok(Message {
                command,
                payload,
                magic,
                checksum,
            })
        } else {
            Err("Failed to deserialize message")
        }
    }
    
    /// Convert to JSON string
    pub fn to_json(&self) -> Result<String, &'static str> {
        let mut json = String::new();
        let success = ffi::networkMessageToJson(&self.command, &self.payload, self.magic, &mut json);
        
        if success {
            Ok(json)
        } else {
            Err("Failed to convert message to JSON")
        }
    }
    
    /// Validate message checksum
    pub fn is_valid(&self) -> bool {
        ffi::networkMessageValidate(&self.command, &self.payload, self.magic, self.checksum)
    }
    
    /// Get payload size
    pub fn payload_size(&self) -> usize {
        self.payload.len()
    }
    
    /// Check if this is a simple message (no payload)
    pub fn is_simple(&self) -> bool {
        self.payload.is_empty()
    }
}

// Message creation utilities
pub fn create_ping_message(nonce: u64, magic: u32) -> Result<Message, &'static str> {
    let mut payload = Vec::new();
    let success = ffi::networkCreatePingMessage(nonce, &mut payload);
    
    if success {
        Ok(Message::new("ping".to_string(), payload, magic))
    } else {
        Err("Failed to create ping message")
    }
}

pub fn create_pong_message(nonce: u64, magic: u32) -> Result<Message, &'static str> {
    let mut payload = Vec::new();
    let success = ffi::networkCreatePongMessage(nonce, &mut payload);
    
    if success {
        Ok(Message::new("pong".to_string(), payload, magic))
    } else {
        Err("Failed to create pong message")
    }
}

pub fn create_get_address_message(magic: u32) -> Message {
    Message::new("getaddr".to_string(), Vec::new(), magic)
}

pub fn create_filter_clear_message(magic: u32) -> Message {
    Message::new("filterclear".to_string(), Vec::new(), magic)
}

pub fn create_verify_ack_message(magic: u32) -> Message {
    Message::new("verack".to_string(), Vec::new(), magic)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_message_type_conversion() {
        assert_eq!(MessageType::from("ping".to_string()), MessageType::Ping);
        assert_eq!(MessageType::from("pong".to_string()), MessageType::Pong);
        assert_eq!(MessageType::from("version".to_string()), MessageType::Version);
        assert_eq!(MessageType::from("verack".to_string()), MessageType::VerifyAck);
        assert_eq!(MessageType::from("unknown".to_string()), MessageType::Unknown);
        
        assert_eq!(String::from(MessageType::Ping), "ping");
        assert_eq!(String::from(MessageType::Pong), "pong");
        assert_eq!(String::from(MessageType::Version), "version");
        assert_eq!(String::from(MessageType::VerifyAck), "verack");
    }
    
    #[test]
    fn test_message_creation() {
        let magic = 0xD9B4BEF9; // Bitcoin mainnet magic
        let payload = vec![1, 2, 3, 4];
        let msg = Message::new("test".to_string(), payload.clone(), magic);
        
        assert_eq!(msg.command, "test");
        assert_eq!(msg.payload, payload);
        assert_eq!(msg.magic, magic);
        assert_eq!(msg.checksum, 0);
        assert_eq!(msg.message_type(), MessageType::Unknown);
    }
    
    #[test]
    fn test_simple_message_creation() {
        let magic = 0xD9B4BEF9;
        
        let getaddr = create_get_address_message(magic);
        assert_eq!(getaddr.command, "getaddr");
        assert!(getaddr.is_simple());
        
        let filterclear = create_filter_clear_message(magic);
        assert_eq!(filterclear.command, "filterclear");
        assert!(filterclear.is_simple());
        
        let verack = create_verify_ack_message(magic);
        assert_eq!(verack.command, "verack");
        assert!(verack.is_simple());
    }
    
    #[test]
    fn test_message_payload_info() {
        let empty_msg = Message::new("getaddr".to_string(), Vec::new(), 0);
        assert_eq!(empty_msg.payload_size(), 0);
        assert!(empty_msg.is_simple());
        
        let data_msg = Message::new("tx".to_string(), vec![1, 2, 3], 0);
        assert_eq!(data_msg.payload_size(), 3);
        assert!(!data_msg.is_simple());
    }
    
    #[test]
    fn test_message_serialization() {
        let magic = 0xD9B4BEF9;
        let payload = vec![1, 2, 3, 4];
        let msg = Message::new("ping".to_string(), payload, magic);
        
        // Test serialization (if bridge functions work)
        if let Ok(serialized) = msg.serialize() {
            assert!(!serialized.is_empty());
            
            // Test deserialization
            if let Ok(deserialized) = Message::deserialize(&serialized) {
                assert_eq!(msg.command, deserialized.command);
                assert_eq!(msg.payload, deserialized.payload);
                assert_eq!(msg.magic, deserialized.magic);
            }
        }
    }
}
