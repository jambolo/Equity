//! Pure-Rust port of the legacy C++ `P2p` namespace.
//!
//! Covers wire-format helpers shared by the network and equity crates:
//! - Bitcoin CompactSize (variable-length integer) and variable-length string
//! - A minimal `Message` envelope (command + payload), matching the legacy `P2p::Message`

#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Message {
    pub command: String,
    pub payload: Vec<u8>,
}

impl Message {
    pub fn new(command: impl Into<String>, payload: impl Into<Vec<u8>>) -> Self {
        Self {
            command: command.into(),
            payload: payload.into(),
        }
    }

    pub fn size(&self) -> usize {
        self.payload.len()
    }
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VarIntResult {
    pub value: u64,
    pub bytes_read: usize,
}

#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VarStringResult {
    pub value: String,
    pub bytes_read: usize,
}

pub fn serialize_var_int(value: u64) -> Vec<u8> {
    let mut data = Vec::new();
    if value < 0xFD {
        data.push(value as u8);
    } else if value <= 0xFFFF {
        data.push(0xFD);
        data.extend_from_slice(&(value as u16).to_le_bytes());
    } else if value <= 0xFFFF_FFFF {
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

pub fn deserialize_var_int(data: &[u8]) -> VarIntResult {
    let fail = VarIntResult {
        value: 0,
        bytes_read: 0,
    };
    let Some(&first) = data.first() else {
        return fail;
    };
    match first {
        0..=0xFC => VarIntResult {
            value: first as u64,
            bytes_read: 1,
        },
        0xFD if data.len() >= 3 => VarIntResult {
            value: u16::from_le_bytes([data[1], data[2]]) as u64,
            bytes_read: 3,
        },
        0xFE if data.len() >= 5 => VarIntResult {
            value: u32::from_le_bytes([data[1], data[2], data[3], data[4]]) as u64,
            bytes_read: 5,
        },
        0xFF if data.len() >= 9 => VarIntResult {
            value: u64::from_le_bytes([
                data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
            ]),
            bytes_read: 9,
        },
        _ => fail,
    }
}

pub fn deserialize_var_string(data: &[u8]) -> VarStringResult {
    let fail = VarStringResult {
        value: String::new(),
        bytes_read: 0,
    };
    let length = deserialize_var_int(data);
    if length.bytes_read == 0 {
        return fail;
    }
    let total = length.bytes_read + length.value as usize;
    if data.len() < total {
        return fail;
    }
    match String::from_utf8(data[length.bytes_read..total].to_vec()) {
        Ok(value) => VarStringResult {
            value,
            bytes_read: total,
        },
        Err(_) => fail,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn var_int_roundtrip() {
        for &v in &[
            0u64,
            1,
            0xFC,
            0xFD,
            0xFFFF,
            0x1_0000,
            0xFFFF_FFFF,
            0x1_0000_0000,
            u64::MAX,
        ] {
            let bytes = serialize_var_int(v);
            let r = deserialize_var_int(&bytes);
            assert_eq!(r.value, v);
            assert_eq!(r.bytes_read, bytes.len());
        }
    }

    #[test]
    fn var_int_short_buffers() {
        assert_eq!(deserialize_var_int(&[]).bytes_read, 0);
        assert_eq!(deserialize_var_int(&[0xFD, 0x00]).bytes_read, 0);
        assert_eq!(deserialize_var_int(&[0xFE, 0x00, 0x00]).bytes_read, 0);
        assert_eq!(deserialize_var_int(&[0xFF, 0x00, 0x00, 0x00]).bytes_read, 0);
    }

    #[test]
    fn var_int_encoding() {
        assert_eq!(serialize_var_int(0), vec![0]);
        assert_eq!(serialize_var_int(0xFC), vec![0xFC]);
        assert_eq!(serialize_var_int(0xFD), vec![0xFD, 0xFD, 0x00]);
        assert_eq!(serialize_var_int(0x1_0000), vec![0xFE, 0x00, 0x00, 0x01, 0x00]);
    }

    #[test]
    fn var_string_roundtrip() {
        let s = "hello world";
        let bytes = serialize_var_string(s);
        let r = deserialize_var_string(&bytes);
        assert_eq!(r.value, s);
        assert_eq!(r.bytes_read, bytes.len());
    }

    #[test]
    fn message_basics() {
        let m = Message::new("ping", vec![1, 2, 3]);
        assert_eq!(m.command, "ping");
        assert_eq!(m.size(), 3);
    }
}
