//! P2P (Peer-to-Peer) functionality
//! 
//! This library provides Rust bindings for the P2P C++ library
//! using the cxx crate for safe FFI.

use std::os::raw::c_char;
use cxx::{UniquePtr, Pin};

#[cxx::bridge]
mod ffi {
    struct MessageHeaderCpp {
        magic: u32,
        type_name: String,
        length: u32,
        checksum: u32,
    }

    struct VarIntResult {
        value: u64,
        bytes_read: usize,
    }

    struct VarStringResult {
        value: String,
        bytes_read: usize,
    }

    unsafe extern "C++" {
        include!("p2p_wrapper.h");
        
        type MessageCpp;
        type PeerCpp;

        // Message functions (simplified to use strings and slices)
        unsafe fn messageCreate(message_type: *const c_char, payload: *const u8, payload_len: usize) -> UniquePtr<MessageCpp>;
        fn messageGetType(message: &MessageCpp) -> String;
        fn messageGetPayload(message: &MessageCpp) -> Vec<u8>;
        fn messageGetSize(message: &MessageCpp) -> usize;
        fn messageToJson(message: &MessageCpp) -> String;

        // Message header functions
        unsafe fn messageHeaderCreate(magic: u32, message_type: *const c_char, length: u32, checksum: u32) -> MessageHeaderCpp;
        unsafe fn messageHeaderFromData(data: *const u8, data_len: usize) -> MessageHeaderCpp;
        fn messageHeaderSerialize(header: &MessageHeaderCpp) -> Vec<u8>;
        fn messageHeaderToJson(header: &MessageHeaderCpp) -> String;

        // Peer functions
        unsafe fn peerCreate(address: *const c_char) -> UniquePtr<PeerCpp>;
        unsafe fn peerFromConnection(address: *const c_char, connection_id: u32) -> UniquePtr<PeerCpp>;
        fn peerGetAddress(peer: &PeerCpp) -> String;
        fn peerGetConnectionStatus(peer: &PeerCpp) -> i32;
        fn peerGetServices(peer: &PeerCpp) -> u64;
        fn peerSetServices(peer: Pin<&mut PeerCpp>, services: u64);
        fn peerGetProtocolVersion(peer: &PeerCpp) -> u32;
        fn peerSetProtocolVersion(peer: Pin<&mut PeerCpp>, version: u32);
        fn peerGetUserAgent(peer: &PeerCpp) -> String;
        unsafe fn peerSetUserAgent(peer: Pin<&mut PeerCpp>, user_agent: *const c_char);
        fn peerGetStartHeight(peer: &PeerCpp) -> u32;
        fn peerSetStartHeight(peer: Pin<&mut PeerCpp>, height: u32);
        fn peerGetLastSeen(peer: &PeerCpp) -> u64;
        fn peerSetLastSeen(peer: Pin<&mut PeerCpp>, timestamp: u64);
        fn peerGetConnectionTime(peer: &PeerCpp) -> u64;
        fn peerConnect(peer: Pin<&mut PeerCpp>) -> bool;
        fn peerDisconnect(peer: Pin<&mut PeerCpp>);
        fn peerSendMessage(peer: Pin<&mut PeerCpp>, message: &MessageCpp) -> bool;
        fn peerHasPendingMessages(peer: &PeerCpp) -> bool;
        fn peerReceiveMessage(peer: Pin<&mut PeerCpp>) -> UniquePtr<MessageCpp>;
        fn peerGetBytesSent(peer: &PeerCpp) -> u64;
        fn peerGetBytesReceived(peer: &PeerCpp) -> u64;
        fn peerGetMessagesSent(peer: &PeerCpp) -> u64;
        fn peerGetMessagesReceived(peer: &PeerCpp) -> u64;
        fn peerToJson(peer: &PeerCpp) -> String;

        // Serialization functions
        fn serializeU16(value: u16) -> Vec<u8>;
        fn serializeU32(value: u32) -> Vec<u8>;
        fn serializeU64(value: u64) -> Vec<u8>;
        fn serializeI16(value: i16) -> Vec<u8>;
        fn serializeI32(value: i32) -> Vec<u8>;
        fn serializeI64(value: i64) -> Vec<u8>;
        fn serializeVarInt(value: u64) -> Vec<u8>;
        unsafe fn serializeVarString(value: *const c_char) -> Vec<u8>;

        unsafe fn deserializeU16(data: *const u8) -> u16;
        unsafe fn deserializeU32(data: *const u8) -> u32;
        unsafe fn deserializeU64(data: *const u8) -> u64;
        unsafe fn deserializeI16(data: *const u8) -> i16;
        unsafe fn deserializeI32(data: *const u8) -> i32;
        unsafe fn deserializeI64(data: *const u8) -> i64;
        unsafe fn deserializeVarInt(data: *const u8, data_len: usize) -> VarIntResult;
        unsafe fn deserializeVarString(data: *const u8, data_len: usize) -> VarStringResult;
    }
}

pub mod message;
pub mod peer;
pub mod serialize;

// Re-export public types for convenience
pub use message::{Message, MessageHeader};
pub use peer::{Peer, PeerServices, ConnectionStatus};
pub use serialize::{Serializer, Deserializer, VarInt, VarString};

// Common types and utilities
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Network {
    Main,
    Test,
    Test3,
}

impl Network {
    /// Get the magic number for this network
    pub fn magic(&self) -> u32 {
        match self {
            Network::Main => 0xD9B4BEF9,   // Bitcoin mainnet magic
            Network::Test => 0xDAB5BFFA,   // Bitcoin testnet magic
            Network::Test3 => 0x0709110B,  // Bitcoin testnet3 magic
        }
    }

    /// Get the default port for this network
    pub fn default_port(&self) -> u16 {
        match self {
            Network::Main => 8333,
            Network::Test => 18333,
            Network::Test3 => 18444,
        }
    }

    /// Get the network name as a string
    pub fn name(&self) -> &'static str {
        match self {
            Network::Main => "mainnet",
            Network::Test => "testnet",
            Network::Test3 => "testnet3",
        }
    }
}

#[derive(Debug)]
pub struct P2pError(pub String);

impl std::fmt::Display for P2pError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "P2P error: {}", self.0)
    }
}

impl std::error::Error for P2pError {}

impl From<std::io::Error> for P2pError {
    fn from(err: std::io::Error) -> Self {
        P2pError(format!("IO error: {}", err))
    }
}

impl From<std::net::AddrParseError> for P2pError {
    fn from(err: std::net::AddrParseError) -> Self {
        P2pError(format!("Address parse error: {}", err))
    }
}

pub type Result<T> = std::result::Result<T, P2pError>;

/// Helper function to create an error
pub fn error(msg: &str) -> P2pError {
    P2pError(msg.to_string())
}

/// Convert a Rust string to a C string pointer for FFI
fn to_c_string(s: &str) -> std::ffi::CString {
    std::ffi::CString::new(s).unwrap_or_else(|_| std::ffi::CString::new("").unwrap())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_network_magic() {
        assert_eq!(Network::Main.magic(), 0xD9B4BEF9);
        assert_eq!(Network::Test.magic(), 0xDAB5BFFA);
        assert_eq!(Network::Test3.magic(), 0x0709110B);
        
        // They should all be different
        assert_ne!(Network::Main.magic(), Network::Test.magic());
        assert_ne!(Network::Main.magic(), Network::Test3.magic());
        assert_ne!(Network::Test.magic(), Network::Test3.magic());
    }

    #[test]
    fn test_network_ports() {
        assert_eq!(Network::Main.default_port(), 8333);
        assert_eq!(Network::Test.default_port(), 18333);
        assert_eq!(Network::Test3.default_port(), 18444);
    }

    #[test]
    fn test_network_names() {
        assert_eq!(Network::Main.name(), "mainnet");
        assert_eq!(Network::Test.name(), "testnet");
        assert_eq!(Network::Test3.name(), "testnet3");
    }

    #[test]
    fn test_error_conversion() {
        let io_err = std::io::Error::new(std::io::ErrorKind::NotFound, "file not found");
        let p2p_err: P2pError = io_err.into();
        assert!(p2p_err.to_string().contains("IO error"));
        
        let addr_err = "invalid_address:port".parse::<std::net::SocketAddr>().unwrap_err();
        let p2p_err: P2pError = addr_err.into();
        assert!(p2p_err.to_string().contains("Address parse error"));
    }
}

