//! P2P Peer functionality
//! 
//! Provides functionality for managing P2P network peers, including connections,
//! state management, and communication.

use crate::ffi::{self, PeerCpp};
use crate::{Result, P2pError, Network, Message, to_c_string};
use std::net::SocketAddr;
use std::time::{Duration, SystemTime, UNIX_EPOCH};
use std::ffi::CString;

/// A P2P network peer
pub struct Peer {
    inner: PeerCpp,
}

/// Connection status of a peer
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ConnectionStatus {
    /// Not connected
    Disconnected,
    /// Connection in progress
    Connecting,
    /// Successfully connected
    Connected,
    /// Connection failed
    Failed,
    /// Connection was disconnected
    Terminated,
}

/// Peer capabilities and features
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PeerServices(pub u64);

impl PeerServices {
    /// Network node (NODE_NETWORK)
    pub const NETWORK: PeerServices = PeerServices(0x01);
    /// UTXO lookup service (NODE_GETUTXO)  
    pub const GETUTXO: PeerServices = PeerServices(0x02);
    /// Bloom filter service (NODE_BLOOM)
    pub const BLOOM: PeerServices = PeerServices(0x04);
    /// Segregated witness (NODE_WITNESS)
    pub const WITNESS: PeerServices = PeerServices(0x08);
    /// Compact filters (NODE_COMPACT_FILTERS)
    pub const COMPACT_FILTERS: PeerServices = PeerServices(0x40);
    /// Network limited (NODE_NETWORK_LIMITED)
    pub const NETWORK_LIMITED: PeerServices = PeerServices(0x400);

    /// Check if the peer has a specific service
    pub fn has_service(&self, service: PeerServices) -> bool {
        (self.0 & service.0) != 0
    }

    /// Add a service to the peer
    pub fn add_service(&mut self, service: PeerServices) {
        self.0 |= service.0;
    }

    /// Remove a service from the peer
    pub fn remove_service(&mut self, service: PeerServices) {
        self.0 &= !service.0;
    }

    /// Check if the peer is a full network node
    pub fn is_full_node(&self) -> bool {
        self.has_service(Self::NETWORK) && !self.has_service(Self::NETWORK_LIMITED)
    }

    /// Check if the peer supports bloom filters
    pub fn supports_bloom(&self) -> bool {
        self.has_service(Self::BLOOM)
    }

    /// Check if the peer supports witness data
    pub fn supports_witness(&self) -> bool {
        self.has_service(Self::WITNESS)
    }
}

impl Peer {
    /// Create a new peer with the given address
    pub fn new(address: SocketAddr) -> Self {
        let addr_str = to_c_string(&address.to_string());
        let inner = unsafe { ffi::peerCreate(addr_str.as_ptr()) };
        Peer { inner }
    }

    /// Create a peer from an existing connection
    pub fn from_connection(address: SocketAddr, connection_id: u32) -> Self {
        let addr_str = to_c_string(&address.to_string());
        let inner = unsafe { ffi::peerFromConnection(addr_str.as_ptr(), connection_id) };
        Peer { inner }
    }

    /// Get the peer's address
    pub fn address(&self) -> Result<SocketAddr> {
        let addr_str = unsafe { ffi::peerGetAddress(&self.inner) };
        addr_str.parse()
            .map_err(|e| P2pError(format!("Invalid peer address: {}", e)))
    }

    /// Get the peer's connection status
    pub fn connection_status(&self) -> ConnectionStatus {
        match unsafe { ffi::peerGetConnectionStatus(&self.inner) } {
            0 => ConnectionStatus::Disconnected,
            1 => ConnectionStatus::Connecting,
            2 => ConnectionStatus::Connected,
            3 => ConnectionStatus::Failed,
            4 => ConnectionStatus::Terminated,
            _ => ConnectionStatus::Failed,
        }
    }

    /// Check if the peer is connected
    pub fn is_connected(&self) -> bool {
        self.connection_status() == ConnectionStatus::Connected
    }

    /// Check if the peer can connect
    pub fn can_connect(&self) -> bool {
        matches!(
            self.connection_status(),
            ConnectionStatus::Disconnected | ConnectionStatus::Failed | ConnectionStatus::Terminated
        )
    }

    /// Get the peer's services
    pub fn services(&self) -> PeerServices {
        PeerServices(unsafe { ffi::peerGetServices(&self.inner) })
    }

    /// Set the peer's services
    pub fn set_services(&mut self, services: PeerServices) {
        unsafe { ffi::peerSetServices(&mut self.inner, services.0) };
    }

    /// Get the peer's protocol version
    pub fn protocol_version(&self) -> u32 {
        unsafe { ffi::peerGetProtocolVersion(&self.inner) }
    }

    /// Set the peer's protocol version
    pub fn set_protocol_version(&mut self, version: u32) {
        unsafe { ffi::peerSetProtocolVersion(&mut self.inner, version) };
    }

    /// Get the peer's user agent string
    pub fn user_agent(&self) -> String {
        unsafe { ffi::peerGetUserAgent(&self.inner) }
    }

    /// Set the peer's user agent string
    pub fn set_user_agent(&mut self, user_agent: &str) {
        let c_agent = to_c_string(user_agent);
        unsafe { ffi::peerSetUserAgent(&mut self.inner, c_agent.as_ptr()) };
    }

    /// Get the peer's starting height
    pub fn start_height(&self) -> u32 {
        unsafe { ffi::peerGetStartHeight(&self.inner) }
    }

    /// Set the peer's starting height
    pub fn set_start_height(&mut self, height: u32) {
        unsafe { ffi::peerSetStartHeight(&mut self.inner, height) };
    }

    /// Get the last time we heard from this peer
    pub fn last_seen(&self) -> SystemTime {
        let timestamp = unsafe { ffi::peerGetLastSeen(&self.inner) };
        UNIX_EPOCH + Duration::from_secs(timestamp)
    }

    /// Update the last seen timestamp to now
    pub fn update_last_seen(&mut self) {
        let now = SystemTime::now()
            .duration_since(UNIX_EPOCH)
            .unwrap_or_default()
            .as_secs();
        unsafe { ffi::peerSetLastSeen(&mut self.inner, now) };
    }

    /// Get the peer's connection time
    pub fn connection_time(&self) -> Option<SystemTime> {
        if self.is_connected() {
            let timestamp = unsafe { ffi::peerGetConnectionTime(&self.inner) };
            if timestamp > 0 {
                Some(UNIX_EPOCH + Duration::from_secs(timestamp))
            } else {
                None
            }
        } else {
            None
        }
    }

    /// Get how long this peer has been connected
    pub fn connection_duration(&self) -> Option<Duration> {
        self.connection_time().map(|start| {
            SystemTime::now().duration_since(start).unwrap_or_default()
        })
    }

    /// Connect to the peer
    pub fn connect(&mut self) -> Result<()> {
        if !self.can_connect() {
            return Err(P2pError("Peer cannot connect in current state".to_string()));
        }
        
        let success = unsafe { ffi::peerConnect(&mut self.inner) };
        if success {
            Ok(())
        } else {
            Err(P2pError("Failed to connect to peer".to_string()))
        }
    }

    /// Disconnect from the peer
    pub fn disconnect(&mut self) {
        unsafe { ffi::peerDisconnect(&mut self.inner) };
    }

    /// Send a message to the peer
    pub fn send_message(&mut self, message: &Message) -> Result<()> {
        if !self.is_connected() {
            return Err(P2pError("Peer is not connected".to_string()));
        }
        
        let success = unsafe { ffi::peerSendMessage(&mut self.inner, &message.inner) };
        if success {
            Ok(())
        } else {
            Err(P2pError("Failed to send message to peer".to_string()))
        }
    }

    /// Check if there are pending messages from this peer
    pub fn has_pending_messages(&self) -> bool {
        unsafe { ffi::peerHasPendingMessages(&self.inner) }
    }

    /// Receive the next message from the peer
    pub fn receive_message(&mut self) -> Option<Message> {
        if self.has_pending_messages() {
            let inner = unsafe { ffi::peerReceiveMessage(&mut self.inner) };
            Some(Message { inner })
        } else {
            None
        }
    }

    /// Get network statistics for this peer
    pub fn bytes_sent(&self) -> u64 {
        unsafe { ffi::peerGetBytesSent(&self.inner) }
    }

    /// Get bytes received from this peer
    pub fn bytes_received(&self) -> u64 {
        unsafe { ffi::peerGetBytesReceived(&self.inner) }
    }

    /// Get the number of messages sent to this peer
    pub fn messages_sent(&self) -> u64 {
        unsafe { ffi::peerGetMessagesSent(&self.inner) }
    }

    /// Get the number of messages received from this peer
    pub fn messages_received(&self) -> u64 {
        unsafe { ffi::peerGetMessagesReceived(&self.inner) }
    }

    /// Check if this peer is considered stale (no recent activity)
    pub fn is_stale(&self, max_age: Duration) -> bool {
        match SystemTime::now().duration_since(self.last_seen()) {
            Ok(age) => age > max_age,
            Err(_) => true, // If last_seen is in the future, consider it stale
        }
    }

    /// Get a summary string for this peer
    pub fn summary(&self) -> String {
        format!(
            "Peer {{ addr: {}, status: {:?}, services: 0x{:X}, version: {}, agent: {} }}",
            self.address().map(|a| a.to_string()).unwrap_or_else(|_| "unknown".to_string()),
            self.connection_status(),
            self.services().0,
            self.protocol_version(),
            self.user_agent()
        )
    }

    /// Convert the peer information to JSON
    pub fn to_json(&self) -> String {
        unsafe { ffi::peerToJson(&self.inner) }
    }
}

impl std::fmt::Debug for Peer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Peer")
            .field("address", &self.address().unwrap_or_else(|_| "0.0.0.0:0".parse().unwrap()))
            .field("status", &self.connection_status())
            .field("services", &format!("0x{:X}", self.services().0))
            .field("version", &self.protocol_version())
            .field("user_agent", &self.user_agent())
            .finish()
    }
}

impl std::fmt::Display for Peer {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.summary())
    }
}

impl std::fmt::Display for PeerServices {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let mut services = Vec::new();
        
        if self.has_service(Self::NETWORK) {
            services.push("NETWORK");
        }
        if self.has_service(Self::GETUTXO) {
            services.push("GETUTXO");
        }
        if self.has_service(Self::BLOOM) {
            services.push("BLOOM");
        }
        if self.has_service(Self::WITNESS) {
            services.push("WITNESS");
        }
        if self.has_service(Self::COMPACT_FILTERS) {
            services.push("COMPACT_FILTERS");
        }
        if self.has_service(Self::NETWORK_LIMITED) {
            services.push("NETWORK_LIMITED");
        }
        
        if services.is_empty() {
            write!(f, "NONE")
        } else {
            write!(f, "{}", services.join(" | "))
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::net::{IpAddr, Ipv4Addr};

    #[test]
    fn test_peer_creation() {
        let addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::new(127, 0, 0, 1)), 8333);
        let peer = Peer::new(addr);
        
        assert_eq!(peer.address().unwrap(), addr);
        assert_eq!(peer.connection_status(), ConnectionStatus::Disconnected);
        assert!(peer.can_connect());
        assert!(!peer.is_connected());
    }

    #[test]
    fn test_peer_services() {
        let mut services = PeerServices(0);
        assert!(!services.has_service(PeerServices::NETWORK));
        
        services.add_service(PeerServices::NETWORK);
        assert!(services.has_service(PeerServices::NETWORK));
        assert!(services.is_full_node());
        
        services.add_service(PeerServices::BLOOM);
        assert!(services.supports_bloom());
        
        services.remove_service(PeerServices::NETWORK);
        assert!(!services.has_service(PeerServices::NETWORK));
        assert!(!services.is_full_node());
    }

    #[test]
    fn test_connection_status() {
        let addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::new(192, 168, 1, 1)), 8333);
        let peer = Peer::new(addr);
        
        assert_eq!(peer.connection_status(), ConnectionStatus::Disconnected);
        assert!(peer.can_connect());
        assert!(!peer.is_connected());
    }

    #[test]
    fn test_peer_staleness() {
        let addr = SocketAddr::new(IpAddr::V4(Ipv4Addr::new(10, 0, 0, 1)), 8333);
        let mut peer = Peer::new(addr);
        
        peer.update_last_seen();
        assert!(!peer.is_stale(Duration::from_secs(60)));
        assert!(peer.is_stale(Duration::from_secs(0)));
    }

    #[test]
    fn test_services_display() {
        let mut services = PeerServices::NETWORK;
        services.add_service(PeerServices::BLOOM);
        services.add_service(PeerServices::WITNESS);
        
        let display = format!("{}", services);
        assert!(display.contains("NETWORK"));
        assert!(display.contains("BLOOM"));
        assert!(display.contains("WITNESS"));
    }
}
