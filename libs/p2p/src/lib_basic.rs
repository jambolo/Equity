//! P2P (Peer-to-Peer) functionality
//! 
//! This library provides Rust bindings for the P2P C++ library

// Basic struct definitions that match the C++ interface
#[derive(Clone, Debug)]
pub struct MessageHeaderCpp {
    pub magic: u32,
    pub type_name: String,
    pub length: u32,
    pub checksum: u32,
}

#[derive(Clone, Debug)]
pub struct VarIntResult {
    pub value: u64,
    pub bytes_read: usize,
}

#[derive(Clone, Debug)]
pub struct VarStringResult {
    pub value: String,
    pub bytes_read: usize,
}

pub struct MessageCpp {
    pub message_type: String,
    pub payload: Vec<u8>,
}

pub struct PeerCpp {
    pub address: String,
    pub status: i32,
    pub services: u64,
    pub protocol_version: u32,
    pub user_agent: String,
    pub start_height: u32,
    pub last_seen: u64,
    pub connection_time: u64,
    pub bytes_sent: u64,
    pub bytes_received: u64,
    pub messages_sent: u64,
    pub messages_received: u64,
}

// These would be the high-level Rust API functions
impl MessageCpp {
    pub fn new(message_type: &str, payload: &[u8]) -> Self {
        Self {
            message_type: message_type.to_string(),
            payload: payload.to_vec(),
        }
    }

    pub fn get_type(&self) -> &str {
        &self.message_type
    }

    pub fn get_payload(&self) -> &[u8] {
        &self.payload
    }

    pub fn get_size(&self) -> usize {
        self.payload.len()
    }

    pub fn to_json(&self) -> String {
        format!("{{\"type\":\"{}\",\"size\":{}}}", self.message_type, self.payload.len())
    }
}

impl PeerCpp {
    pub fn new(address: &str) -> Self {
        Self {
            address: address.to_string(),
            status: 0,
            services: 0,
            protocol_version: 0,
            user_agent: String::new(),
            start_height: 0,
            last_seen: 0,
            connection_time: 0,
            bytes_sent: 0,
            bytes_received: 0,
            messages_sent: 0,
            messages_received: 0,
        }
    }

    pub fn get_address(&self) -> &str {
        &self.address
    }

    pub fn get_connection_status(&self) -> i32 {
        self.status
    }

    pub fn get_services(&self) -> u64 {
        self.services
    }

    pub fn set_services(&mut self, services: u64) {
        self.services = services;
    }

    pub fn get_protocol_version(&self) -> u32 {
        self.protocol_version
    }

    pub fn set_protocol_version(&mut self, version: u32) {
        self.protocol_version = version;
    }

    pub fn get_user_agent(&self) -> &str {
        &self.user_agent
    }

    pub fn set_user_agent(&mut self, user_agent: &str) {
        self.user_agent = user_agent.to_string();
    }

    pub fn get_start_height(&self) -> u32 {
        self.start_height
    }

    pub fn set_start_height(&mut self, height: u32) {
        self.start_height = height;
    }

    pub fn get_last_seen(&self) -> u64 {
        self.last_seen
    }

    pub fn set_last_seen(&mut self, timestamp: u64) {
        self.last_seen = timestamp;
    }

    pub fn get_connection_time(&self) -> u64 {
        self.connection_time
    }

    pub fn connect(&mut self) -> bool {
        self.status = 2; // connected
        true
    }

    pub fn disconnect(&mut self) {
        self.status = 0; // disconnected
    }

    pub fn send_message(&mut self, message: &MessageCpp) -> bool {
        if self.status == 2 { // connected
            self.bytes_sent += message.payload.len() as u64;
            self.messages_sent += 1;
            true
        } else {
            false
        }
    }

    pub fn has_pending_messages(&self) -> bool {
        false // stub
    }

    pub fn receive_message(&mut self) -> Option<MessageCpp> {
        None // stub
    }

    pub fn get_bytes_sent(&self) -> u64 {
        self.bytes_sent
    }

    pub fn get_bytes_received(&self) -> u64 {
        self.bytes_received
    }

    pub fn get_messages_sent(&self) -> u64 {
        self.messages_sent
    }

    pub fn get_messages_received(&self) -> u64 {
        self.messages_received
    }

    pub fn to_json(&self) -> String {
        format!("{{\"address\":\"{}\",\"status\":{},\"services\":{}}}", 
                self.address, self.status, self.services)
    }
}

// Serialization functions
pub fn serialize_u16(value: u16) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_u32(value: u32) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_u64(value: u64) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_i16(value: i16) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_i32(value: i32) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_i64(value: i64) -> Vec<u8> {
    value.to_le_bytes().to_vec()
}

pub fn serialize_var_int(value: u64) -> Vec<u8> {
    let mut data = Vec::new();
    
    if value < 0xFD {
        data.push(value as u8);
    } else if value <= 0xFFFF {
        data.push(0xFD);
        data.extend_from_slice(&(value as u16).to_le_bytes());
    } else if value <= 0xFFFFFFFF {
        data.push(0xFE);
        data.extend_from_slice(&(value as u32).to_le_bytes());
    } else {
        data.push(0xFF);
        data.extend_from_slice(&value.to_le_bytes());
    }
    
    data
}

pub fn serialize_var_string(value: &str) -> Vec<u8> {
    let mut data = serialize_var_int(value.len() as u64);
    data.extend_from_slice(value.as_bytes());
    data
}

pub fn deserialize_u16(data: &[u8]) -> Option<u16> {
    if data.len() >= 2 {
        Some(u16::from_le_bytes([data[0], data[1]]))
    } else {
        None
    }
}

pub fn deserialize_u32(data: &[u8]) -> Option<u32> {
    if data.len() >= 4 {
        Some(u32::from_le_bytes([data[0], data[1], data[2], data[3]]))
    } else {
        None
    }
}

pub fn deserialize_u64(data: &[u8]) -> Option<u64> {
    if data.len() >= 8 {
        Some(u64::from_le_bytes([data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7]]))
    } else {
        None
    }
}

pub fn deserialize_var_int(data: &[u8]) -> VarIntResult {
    if data.is_empty() {
        return VarIntResult { value: 0, bytes_read: 0 };
    }
    
    let first_byte = data[0];
    
    if first_byte < 0xFD {
        VarIntResult { value: first_byte as u64, bytes_read: 1 }
    } else if first_byte == 0xFD {
        if data.len() >= 3 {
            let value = u16::from_le_bytes([data[1], data[2]]) as u64;
            VarIntResult { value, bytes_read: 3 }
        } else {
            VarIntResult { value: 0, bytes_read: 0 }
        }
    } else if first_byte == 0xFE {
        if data.len() >= 5 {
            let value = u32::from_le_bytes([data[1], data[2], data[3], data[4]]) as u64;
            VarIntResult { value, bytes_read: 5 }
        } else {
            VarIntResult { value: 0, bytes_read: 0 }
        }
    } else if first_byte == 0xFF {
        if data.len() >= 9 {
            let value = u64::from_le_bytes([data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8]]);
            VarIntResult { value, bytes_read: 9 }
        } else {
            VarIntResult { value: 0, bytes_read: 0 }
        }
    } else {
        VarIntResult { value: 0, bytes_read: 0 }
    }
}

pub fn deserialize_var_string(data: &[u8]) -> VarStringResult {
    let length_result = deserialize_var_int(data);
    if length_result.bytes_read == 0 {
        return VarStringResult { value: String::new(), bytes_read: 0 };
    }
    
    let string_length = length_result.value as usize;
    let total_bytes_needed = length_result.bytes_read + string_length;
    
    if data.len() >= total_bytes_needed {
        let string_bytes = &data[length_result.bytes_read..total_bytes_needed];
        if let Ok(string_value) = String::from_utf8(string_bytes.to_vec()) {
            VarStringResult { value: string_value, bytes_read: total_bytes_needed }
        } else {
            VarStringResult { value: String::new(), bytes_read: 0 }
        }
    } else {
        VarStringResult { value: String::new(), bytes_read: 0 }
    }
}
