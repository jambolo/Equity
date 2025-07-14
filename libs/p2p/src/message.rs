//! P2P Message functionality
//! 
//! Provides functionality for creating, validating, and converting P2P network messages.

use crate::ffi::{self, MessageCpp, MessageHeaderCpp};
use crate::{Result, P2pError, Network, to_c_string};
use std::ffi::CString;

/// A P2P network message
pub struct Message {
    inner: MessageCpp,
}

/// A P2P network message header
pub struct MessageHeader {
    inner: MessageHeaderCpp,
}

impl Message {
    /// Create a new message with the given type and payload
    pub fn new(message_type: &str, payload: &[u8]) -> Self {
        let c_type = to_c_string(message_type);
        let inner = unsafe {
            ffi::messageCreate(c_type.as_ptr(), payload.as_ptr(), payload.len())
        };
        Message { inner }
    }

    /// Get the message type
    pub fn message_type(&self) -> String {
        unsafe { ffi::messageGetType(&self.inner) }
    }

    /// Get the message payload
    pub fn payload(&self) -> Vec<u8> {
        unsafe { ffi::messageGetPayload(&self.inner) }
    }

    /// Get the size of the payload
    pub fn size(&self) -> usize {
        unsafe { ffi::messageGetSize(&self.inner) }
    }

    /// Check if the message is empty
    pub fn is_empty(&self) -> bool {
        self.size() == 0
    }
}

impl MessageHeader {
    /// Magic number constants
    pub const MAGIC_MAIN: u32 = 0xD9B4BEF9;
    pub const MAGIC_TEST: u32 = 0xDAB5BFFA;
    pub const MAGIC_TEST3: u32 = 0x0709110B;

    /// Field sizes
    pub const MAGIC_SIZE: usize = 4;
    pub const TYPE_SIZE: usize = 12;
    pub const LENGTH_SIZE: usize = 4;
    pub const CHECKSUM_SIZE: usize = 4;

    /// Create a new message header
    pub fn new(magic: u32, message_type: &str, length: u32, checksum: u32) -> Self {
        let c_type = to_c_string(message_type);
        let inner = unsafe {
            ffi::messageHeaderCreate(magic, c_type.as_ptr(), length, checksum)
        };
        MessageHeader { inner }
    }

    /// Create a message header for a specific network
    pub fn for_network(network: Network, message_type: &str, length: u32, checksum: u32) -> Self {
        Self::new(network.magic(), message_type, length, checksum)
    }

    /// Create a message header from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        if data.len() < Self::header_size() {
            return Err(P2pError("Data too short for message header".to_string()));
        }
        
        let inner = unsafe {
            ffi::messageHeaderFromData(data.as_ptr(), data.len())
        };
        Ok(MessageHeader { inner })
    }

    /// Serialize the header to binary data
    pub fn serialize(&self) -> Vec<u8> {
        unsafe { ffi::messageHeaderSerialize(&self.inner) }
    }

    /// Convert the header to JSON
    pub fn to_json(&self) -> String {
        unsafe { ffi::messageHeaderToJson(&self.inner) }
    }

    /// Get the magic number
    pub fn magic(&self) -> u32 {
        self.inner.magic
    }

    /// Get the message type
    pub fn message_type(&self) -> &str {
        &self.inner.type_name
    }

    /// Get the payload length
    pub fn length(&self) -> u32 {
        self.inner.length
    }

    /// Get the checksum
    pub fn checksum(&self) -> u32 {
        self.inner.checksum
    }

    /// Get the total size of a message header
    pub const fn header_size() -> usize {
        Self::MAGIC_SIZE + Self::TYPE_SIZE + Self::LENGTH_SIZE + Self::CHECKSUM_SIZE
    }

    /// Validate the magic number for a specific network
    pub fn validate_magic(&self, network: Network) -> bool {
        self.magic() == network.magic()
    }

    /// Determine the network based on magic number
    pub fn detect_network(&self) -> Option<Network> {
        match self.magic() {
            Self::MAGIC_MAIN => Some(Network::Main),
            Self::MAGIC_TEST => Some(Network::Test),
            Self::MAGIC_TEST3 => Some(Network::Test3),
            _ => None,
        }
    }
}

impl std::fmt::Debug for Message {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Message")
            .field("type", &self.message_type())
            .field("size", &self.size())
            .finish()
    }
}

impl std::fmt::Debug for MessageHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("MessageHeader")
            .field("magic", &format!("0x{:08X}", self.magic()))
            .field("type", &self.message_type())
            .field("length", &self.length())
            .field("checksum", &format!("0x{:08X}", self.checksum()))
            .finish()
    }
}

impl std::fmt::Display for MessageHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "MessageHeader {{ magic: 0x{:08X}, type: {}, length: {}, checksum: 0x{:08X} }}", 
               self.magic(), self.message_type(), self.length(), self.checksum())
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_message_creation() {
        let payload = b"Hello, P2P!";
        let message = Message::new("test", payload);
        
        assert_eq!(message.message_type(), "test");
        assert_eq!(message.size(), payload.len());
        assert!(!message.is_empty());
    }

    #[test]
    fn test_message_header_creation() {
        let header = MessageHeader::new(
            MessageHeader::MAGIC_MAIN,
            "version",
            100,
            0x12345678
        );
        
        assert_eq!(header.magic(), MessageHeader::MAGIC_MAIN);
        assert_eq!(header.message_type(), "version");
        assert_eq!(header.length(), 100);
        assert_eq!(header.checksum(), 0x12345678);
    }

    #[test]
    fn test_network_header_creation() {
        let header = MessageHeader::for_network(
            Network::Main,
            "addr",
            50,
            0xABCDEF00
        );
        
        assert!(header.validate_magic(Network::Main));
        assert!(!header.validate_magic(Network::Test));
        assert_eq!(header.detect_network(), Some(Network::Main));
    }

    #[test]
    fn test_header_size() {
        assert_eq!(MessageHeader::header_size(), 24); // 4 + 12 + 4 + 4
    }

    #[test]
    fn test_empty_message() {
        let message = Message::new("ping", &[]);
        assert!(message.is_empty());
        assert_eq!(message.size(), 0);
    }
}
