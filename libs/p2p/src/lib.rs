//! Bitcoin P2P wire-format primitives shared by the `network` and `equity`
//! crates.
//!
//! Provides:
//!
//! - [`serialize_var_int`] / [`deserialize_var_int`] — Bitcoin CompactSize
//!   (variable-length integer) encoding. 1, 3, 5, or 9 bytes depending on
//!   magnitude.
//! - [`serialize_var_string`] / [`deserialize_var_string`] — CompactSize
//!   length prefix followed by raw UTF-8 bytes.
//! - [`Message`] — a minimal command-plus-payload envelope used internally by
//!   the equity crate. The full Bitcoin wire envelope (with magic and
//!   checksum) lives in the `network` crate.
//!
//! Deserializers return a result type carrying both the decoded value and the
//! number of bytes consumed; `bytes_read == 0` signals a short or malformed
//! input rather than a panic.
//!
//! # Examples
//!
//! Encode and decode a varint:
//!
//! ```
//! use p2p::{serialize_var_int, deserialize_var_int};
//!
//! let bytes = serialize_var_int(0xFFFF);
//! assert_eq!(bytes, vec![0xFD, 0xFF, 0xFF]);
//!
//! let r = deserialize_var_int(&bytes);
//! assert_eq!(r.value, 0xFFFF);
//! assert_eq!(r.bytes_read, bytes.len());
//! ```
//!
//! Encode a CompactSize-prefixed string:
//!
//! ```
//! use p2p::{serialize_var_string, deserialize_var_string};
//!
//! let bytes = serialize_var_string("hello");
//! let r = deserialize_var_string(&bytes);
//! assert_eq!(r.value, "hello");
//! ```

/// Minimal command-plus-payload envelope used internally by the equity crate.
#[derive(Clone, Debug, Default, PartialEq, Eq)]
pub struct Message {
    /// ASCII command name (e.g. `"ping"`).
    pub command: String,
    /// Raw payload bytes.
    pub payload: Vec<u8>,
}

impl Message {
    /// Construct a `Message` from a command and payload.
    pub fn new(command: impl Into<String>, payload: impl Into<Vec<u8>>) -> Self {
        Self {
            command: command.into(),
            payload: payload.into(),
        }
    }

    /// Length of the payload in bytes.
    pub fn size(&self) -> usize {
        self.payload.len()
    }
}

/// Result of [`deserialize_var_int`]: parsed value and the number of bytes consumed
/// (`0` indicates a short or malformed buffer).
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VarIntResult {
    /// Decoded varint value.
    pub value: u64,
    /// Number of input bytes consumed, or `0` on failure.
    pub bytes_read: usize,
}

/// Result of [`deserialize_var_string`]: parsed string and bytes consumed
/// (`0` indicates a short, malformed, or non-UTF-8 buffer).
#[derive(Clone, Debug, PartialEq, Eq)]
pub struct VarStringResult {
    /// Decoded string contents.
    pub value: String,
    /// Number of input bytes consumed, or `0` on failure.
    pub bytes_read: usize,
}

/// Encode `value` as a Bitcoin CompactSize varint (1, 3, 5, or 9 bytes).
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

/// Encode `value` as a CompactSize length prefix followed by raw UTF-8 bytes.
pub fn serialize_var_string(value: &str) -> Vec<u8> {
    let mut data = serialize_var_int(value.len() as u64);
    data.extend_from_slice(value.as_bytes());
    data
}

/// Decode a Bitcoin CompactSize varint from the start of `data`.
///
/// On short or malformed input, returns a result with `bytes_read == 0`.
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

/// Decode a CompactSize-prefixed UTF-8 string from the start of `data`.
///
/// On short, malformed, or non-UTF-8 input, returns a result with `bytes_read == 0`.
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

    #[test]
    fn var_string_empty() {
        let bytes = serialize_var_string("");
        assert_eq!(bytes, vec![0]);
        let r = deserialize_var_string(&bytes);
        assert_eq!(r.value, "");
        assert_eq!(r.bytes_read, 1);
    }

    #[test]
    fn var_string_truncated_payload() {
        // Length=5 but only 2 bytes follow.
        let bytes = vec![5u8, b'a', b'b'];
        let r = deserialize_var_string(&bytes);
        assert_eq!(r.bytes_read, 0);
    }

    #[test]
    fn var_string_invalid_utf8_rejected() {
        let bytes = vec![3u8, 0xFFu8, 0xFE, 0xFD];
        let r = deserialize_var_string(&bytes);
        assert_eq!(r.bytes_read, 0);
    }

    #[test]
    fn var_int_encoding_boundaries() {
        // 0xFD requires the 3-byte form.
        assert_eq!(serialize_var_int(0xFD), vec![0xFD, 0xFD, 0x00]);
        // 0xFFFF still fits 3-byte form.
        assert_eq!(serialize_var_int(0xFFFF), vec![0xFD, 0xFF, 0xFF]);
        // 0x10000 starts 5-byte form.
        assert_eq!(serialize_var_int(0x10000), vec![0xFE, 0x00, 0x00, 0x01, 0x00]);
        // u32::MAX still fits 5-byte form.
        assert_eq!(
            serialize_var_int(0xFFFF_FFFF),
            vec![0xFE, 0xFF, 0xFF, 0xFF, 0xFF]
        );
        // 0x1_0000_0000 starts 9-byte form.
        assert_eq!(
            serialize_var_int(0x1_0000_0000),
            vec![0xFF, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00]
        );
    }

    #[test]
    fn message_default() {
        let m: Message = Default::default();
        assert_eq!(m.command, "");
        assert_eq!(m.size(), 0);
    }

    proptest::proptest! {
        #[test]
        fn prop_var_int_round_trip(v: u64) {
            let bytes = serialize_var_int(v);
            let r = deserialize_var_int(&bytes);
            proptest::prop_assert_eq!(r.value, v);
            proptest::prop_assert_eq!(r.bytes_read, bytes.len());
        }

        #[test]
        fn prop_var_string_round_trip(s in "[ -~]{0,500}") {
            let bytes = serialize_var_string(&s);
            let r = deserialize_var_string(&bytes);
            proptest::prop_assert_eq!(r.value, s);
            proptest::prop_assert_eq!(r.bytes_read, bytes.len());
        }

        #[test]
        fn prop_var_int_length_matches_value_range(v: u64) {
            let n = serialize_var_int(v).len();
            let expected = match v {
                0..=0xFC => 1,
                0xFD..=0xFFFF => 3,
                0x10000..=0xFFFFFFFF => 5,
                _ => 9,
            };
            proptest::prop_assert_eq!(n, expected);
        }
    }
}
