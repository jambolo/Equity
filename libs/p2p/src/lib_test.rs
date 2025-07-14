use p2p::*;

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_message_creation() {
        let message = MessageCpp::new("version", b"test payload");
        assert_eq!(message.get_type(), "version");
        assert_eq!(message.get_payload(), b"test payload");
        assert_eq!(message.get_size(), 12);
    }

    #[test]
    fn test_peer_creation() {
        let mut peer = PeerCpp::new("127.0.0.1:8333");
        assert_eq!(peer.get_address(), "127.0.0.1:8333");
        assert_eq!(peer.get_connection_status(), 0); // disconnected
        
        // Test connection
        assert!(peer.connect());
        assert_eq!(peer.get_connection_status(), 2); // connected
        
        // Test message sending
        let message = MessageCpp::new("ping", b"");
        assert!(peer.send_message(&message));
        assert_eq!(peer.get_messages_sent(), 1);
        
        peer.disconnect();
        assert_eq!(peer.get_connection_status(), 0); // disconnected
    }

    #[test]
    fn test_serialization() {
        // Test basic integer serialization
        assert_eq!(serialize_u32(0x12345678), vec![0x78, 0x56, 0x34, 0x12]);
        
        // Test var_int serialization
        assert_eq!(serialize_var_int(42), vec![42]);
        assert_eq!(serialize_var_int(253), vec![0xFD, 0xFD, 0x00]);
        
        // Test var_string serialization
        let data = serialize_var_string("hello");
        assert_eq!(data[0], 5); // length
        assert_eq!(&data[1..], b"hello");
    }

    #[test]
    fn test_deserialization() {
        // Test var_int deserialization
        let result = deserialize_var_int(&[42]);
        assert_eq!(result.value, 42);
        assert_eq!(result.bytes_read, 1);
        
        let result = deserialize_var_int(&[0xFD, 0xFD, 0x00]);
        assert_eq!(result.value, 253);
        assert_eq!(result.bytes_read, 3);
        
        // Test var_string deserialization
        let data = vec![5, b'h', b'e', b'l', b'l', b'o'];
        let result = deserialize_var_string(&data);
        assert_eq!(result.value, "hello");
        assert_eq!(result.bytes_read, 6);
    }

    #[test]
    fn test_peer_services() {
        let mut peer = PeerCpp::new("127.0.0.1:8333");
        assert_eq!(peer.get_services(), 0);
        
        peer.set_services(0x01); // NODE_NETWORK
        assert_eq!(peer.get_services(), 0x01);
        
        peer.set_protocol_version(70015);
        assert_eq!(peer.get_protocol_version(), 70015);
        
        peer.set_user_agent("/EquityNode:0.1.0/");
        assert_eq!(peer.get_user_agent(), "/EquityNode:0.1.0/");
    }
}
