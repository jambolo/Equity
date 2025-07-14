//! P2P Serialization functionality
//! 
//! Provides comprehensive serialization and deserialization functionality for P2P network
//! protocol types, including integers, strings, variable-length data, and protocol messages.

use crate::ffi::{self};
use crate::{Result, P2pError};
use std::io::{Read, Write};

/// Serializer for P2P protocol data
pub struct Serializer {
    buffer: Vec<u8>,
}

/// Deserializer for P2P protocol data
pub struct Deserializer<'a> {
    data: &'a [u8],
    position: usize,
}

/// Variable-length integer (VarInt) used in Bitcoin protocol
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct VarInt(pub u64);

/// Variable-length string used in Bitcoin protocol
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct VarString(pub String);

impl Serializer {
    /// Create a new serializer
    pub fn new() -> Self {
        Self {
            buffer: Vec::new(),
        }
    }

    /// Create a serializer with pre-allocated capacity
    pub fn with_capacity(capacity: usize) -> Self {
        Self {
            buffer: Vec::with_capacity(capacity),
        }
    }

    /// Get the current buffer contents
    pub fn data(&self) -> &[u8] {
        &self.buffer
    }

    /// Get the current size of the buffer
    pub fn len(&self) -> usize {
        self.buffer.len()
    }

    /// Check if the buffer is empty
    pub fn is_empty(&self) -> bool {
        self.buffer.is_empty()
    }

    /// Clear the buffer
    pub fn clear(&mut self) {
        self.buffer.clear();
    }

    /// Take ownership of the buffer
    pub fn into_vec(self) -> Vec<u8> {
        self.buffer
    }

    /// Write a byte
    pub fn write_u8(&mut self, value: u8) {
        self.buffer.push(value);
    }

    /// Write a 16-bit unsigned integer (little-endian)
    pub fn write_u16(&mut self, value: u16) {
        let data = unsafe { ffi::serializeU16(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a 32-bit unsigned integer (little-endian)
    pub fn write_u32(&mut self, value: u32) {
        let data = unsafe { ffi::serializeU32(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a 64-bit unsigned integer (little-endian)
    pub fn write_u64(&mut self, value: u64) {
        let data = unsafe { ffi::serializeU64(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a 16-bit signed integer (little-endian)
    pub fn write_i16(&mut self, value: i16) {
        let data = unsafe { ffi::serializeI16(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a 32-bit signed integer (little-endian)
    pub fn write_i32(&mut self, value: i32) {
        let data = unsafe { ffi::serializeI32(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a 64-bit signed integer (little-endian)
    pub fn write_i64(&mut self, value: i64) {
        let data = unsafe { ffi::serializeI64(value) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a boolean value (as a single byte)
    pub fn write_bool(&mut self, value: bool) {
        self.write_u8(if value { 1 } else { 0 });
    }

    /// Write a variable-length integer
    pub fn write_varint(&mut self, varint: VarInt) {
        let data = unsafe { ffi::serializeVarInt(varint.0) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write a variable-length string
    pub fn write_varstring(&mut self, varstring: &VarString) {
        use crate::to_c_string;
        let c_str = to_c_string(&varstring.0);
        let data = unsafe { ffi::serializeVarString(c_str.as_ptr()) };
        self.buffer.extend_from_slice(&data);
    }

    /// Write raw bytes
    pub fn write_bytes(&mut self, bytes: &[u8]) {
        self.buffer.extend_from_slice(bytes);
    }

    /// Write bytes with a length prefix
    pub fn write_bytes_with_length(&mut self, bytes: &[u8]) {
        self.write_varint(VarInt(bytes.len() as u64));
        self.write_bytes(bytes);
    }

    /// Write a fixed-size array
    pub fn write_array<const N: usize>(&mut self, array: &[u8; N]) {
        self.buffer.extend_from_slice(array);
    }

    /// Write a string with a length prefix
    pub fn write_string(&mut self, s: &str) {
        let bytes = s.as_bytes();
        self.write_bytes_with_length(bytes);
    }
}

impl<'a> Deserializer<'a> {
    /// Create a new deserializer from data
    pub fn new(data: &'a [u8]) -> Self {
        Self {
            data,
            position: 0,
        }
    }

    /// Get the current position
    pub fn position(&self) -> usize {
        self.position
    }

    /// Get the remaining bytes
    pub fn remaining(&self) -> usize {
        self.data.len().saturating_sub(self.position)
    }

    /// Check if we're at the end of the data
    pub fn is_at_end(&self) -> bool {
        self.position >= self.data.len()
    }

    /// Peek at the next byte without advancing position
    pub fn peek_u8(&self) -> Result<u8> {
        if self.position < self.data.len() {
            Ok(self.data[self.position])
        } else {
            Err(P2pError("Unexpected end of data".to_string()))
        }
    }

    /// Skip bytes
    pub fn skip(&mut self, count: usize) -> Result<()> {
        if self.position + count <= self.data.len() {
            self.position += count;
            Ok(())
        } else {
            Err(P2pError("Cannot skip beyond end of data".to_string()))
        }
    }

    /// Read a byte
    pub fn read_u8(&mut self) -> Result<u8> {
        if self.position < self.data.len() {
            let value = self.data[self.position];
            self.position += 1;
            Ok(value)
        } else {
            Err(P2pError("Unexpected end of data".to_string()))
        }
    }

    /// Read a 16-bit unsigned integer (little-endian)
    pub fn read_u16(&mut self) -> Result<u16> {
        if self.position + 2 <= self.data.len() {
            let value = unsafe { ffi::deserializeU16(self.data.as_ptr().add(self.position)) };
            self.position += 2;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for u16".to_string()))
        }
    }

    /// Read a 32-bit unsigned integer (little-endian)
    pub fn read_u32(&mut self) -> Result<u32> {
        if self.position + 4 <= self.data.len() {
            let value = unsafe { ffi::deserializeU32(self.data.as_ptr().add(self.position)) };
            self.position += 4;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for u32".to_string()))
        }
    }

    /// Read a 64-bit unsigned integer (little-endian)
    pub fn read_u64(&mut self) -> Result<u64> {
        if self.position + 8 <= self.data.len() {
            let value = unsafe { ffi::deserializeU64(self.data.as_ptr().add(self.position)) };
            self.position += 8;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for u64".to_string()))
        }
    }

    /// Read a 16-bit signed integer (little-endian)
    pub fn read_i16(&mut self) -> Result<i16> {
        if self.position + 2 <= self.data.len() {
            let value = unsafe { ffi::deserializeI16(self.data.as_ptr().add(self.position)) };
            self.position += 2;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for i16".to_string()))
        }
    }

    /// Read a 32-bit signed integer (little-endian)
    pub fn read_i32(&mut self) -> Result<i32> {
        if self.position + 4 <= self.data.len() {
            let value = unsafe { ffi::deserializeI32(self.data.as_ptr().add(self.position)) };
            self.position += 4;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for i32".to_string()))
        }
    }

    /// Read a 64-bit signed integer (little-endian)
    pub fn read_i64(&mut self) -> Result<i64> {
        if self.position + 8 <= self.data.len() {
            let value = unsafe { ffi::deserializeI64(self.data.as_ptr().add(self.position)) };
            self.position += 8;
            Ok(value)
        } else {
            Err(P2pError("Not enough data for i64".to_string()))
        }
    }

    /// Read a boolean value (from a single byte)
    pub fn read_bool(&mut self) -> Result<bool> {
        Ok(self.read_u8()? != 0)
    }

    /// Read a variable-length integer
    pub fn read_varint(&mut self) -> Result<VarInt> {
        let remaining = self.data.len() - self.position;
        if remaining == 0 {
            return Err(P2pError("No data remaining for VarInt".to_string()));
        }
        
        let result = unsafe {
            ffi::deserializeVarInt(
                self.data.as_ptr().add(self.position), 
                remaining
            )
        };
        
        if result.bytes_read == 0 {
            return Err(P2pError("Failed to read VarInt".to_string()));
        }
        
        self.position += result.bytes_read;
        Ok(VarInt(result.value))
    }

    /// Read a variable-length string
    pub fn read_varstring(&mut self) -> Result<VarString> {
        let remaining = self.data.len() - self.position;
        if remaining == 0 {
            return Err(P2pError("No data remaining for VarString".to_string()));
        }
        
        let result = unsafe {
            ffi::deserializeVarString(
                self.data.as_ptr().add(self.position), 
                remaining
            )
        };
        
        if result.bytes_read == 0 {
            return Err(P2pError("Failed to read VarString".to_string()));
        }
        
        self.position += result.bytes_read;
        Ok(VarString(result.value))
    }

    /// Read a fixed number of bytes
    pub fn read_bytes(&mut self, count: usize) -> Result<Vec<u8>> {
        if self.position + count <= self.data.len() {
            let bytes = self.data[self.position..self.position + count].to_vec();
            self.position += count;
            Ok(bytes)
        } else {
            Err(P2pError(format!("Not enough data: need {}, have {}", count, self.remaining())))
        }
    }

    /// Read bytes with a length prefix
    pub fn read_bytes_with_length(&mut self) -> Result<Vec<u8>> {
        let length = self.read_varint()?.0 as usize;
        self.read_bytes(length)
    }

    /// Read a fixed-size array
    pub fn read_array<const N: usize>(&mut self) -> Result<[u8; N]> {
        if self.position + N <= self.data.len() {
            let mut array = [0u8; N];
            array.copy_from_slice(&self.data[self.position..self.position + N]);
            self.position += N;
            Ok(array)
        } else {
            Err(P2pError(format!("Not enough data for array of size {}", N)))
        }
    }

    /// Read a string with a length prefix
    pub fn read_string(&mut self) -> Result<String> {
        let bytes = self.read_bytes_with_length()?;
        String::from_utf8(bytes)
            .map_err(|e| P2pError(format!("Invalid UTF-8 string: {}", e)))
    }

    /// Read the remaining bytes
    pub fn read_remaining(&mut self) -> Vec<u8> {
        let bytes = self.data[self.position..].to_vec();
        self.position = self.data.len();
        bytes
    }
}

impl VarInt {
    /// Create a new VarInt
    pub fn new(value: u64) -> Self {
        VarInt(value)
    }

    /// Get the value
    pub fn value(&self) -> u64 {
        self.0
    }

    /// Get the serialized size of this VarInt
    pub fn serialized_size(&self) -> usize {
        match self.0 {
            0..=0xFC => 1,
            0xFD..=0xFFFF => 3,
            0x10000..=0xFFFFFFFF => 5,
            _ => 9,
        }
    }

    /// The maximum value that can be stored in a VarInt
    pub const MAX: u64 = 0xFFFFFFFFFFFFFFFF;
}

impl VarString {
    /// Create a new VarString
    pub fn new(value: String) -> Self {
        VarString(value)
    }

    /// Get the string value
    pub fn value(&self) -> &str {
        &self.0
    }

    /// Get the serialized size of this VarString
    pub fn serialized_size(&self) -> usize {
        let length = self.0.len() as u64;
        VarInt(length).serialized_size() + self.0.len()
    }

    /// Check if the string is empty
    pub fn is_empty(&self) -> bool {
        self.0.is_empty()
    }
}

impl Default for Serializer {
    fn default() -> Self {
        Self::new()
    }
}

impl Write for Serializer {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        self.buffer.extend_from_slice(buf);
        Ok(buf.len())
    }

    fn flush(&mut self) -> std::io::Result<()> {
        Ok(())
    }
}

impl From<u64> for VarInt {
    fn from(value: u64) -> Self {
        VarInt(value)
    }
}

impl From<VarInt> for u64 {
    fn from(varint: VarInt) -> Self {
        varint.0
    }
}

impl From<String> for VarString {
    fn from(value: String) -> Self {
        VarString(value)
    }
}

impl From<&str> for VarString {
    fn from(value: &str) -> Self {
        VarString(value.to_string())
    }
}

impl From<VarString> for String {
    fn from(varstring: VarString) -> Self {
        varstring.0
    }
}

impl std::fmt::Display for VarInt {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

impl std::fmt::Display for VarString {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.0)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_serializer_basic_types() {
        let mut serializer = Serializer::new();
        
        serializer.write_u8(0xFF);
        serializer.write_u16(0x1234);
        serializer.write_u32(0x12345678);
        serializer.write_u64(0x123456789ABCDEF0);
        
        assert_eq!(serializer.len(), 1 + 2 + 4 + 8);
        assert!(!serializer.is_empty());
    }

    #[test]
    fn test_deserializer_basic_types() {
        let data = vec![0xFF, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 0xF0, 0xDE, 0xBC, 0x9A, 0x78, 0x56, 0x34, 0x12];
        let mut deserializer = Deserializer::new(&data);
        
        assert_eq!(deserializer.read_u8().unwrap(), 0xFF);
        assert_eq!(deserializer.read_u16().unwrap(), 0x1234);
        assert_eq!(deserializer.read_u32().unwrap(), 0x12345678);
        assert_eq!(deserializer.read_u64().unwrap(), 0x123456789ABCDEF0);
        
        assert!(deserializer.is_at_end());
    }

    #[test]
    fn test_varint_sizes() {
        assert_eq!(VarInt(0).serialized_size(), 1);
        assert_eq!(VarInt(0xFC).serialized_size(), 1);
        assert_eq!(VarInt(0xFD).serialized_size(), 3);
        assert_eq!(VarInt(0xFFFF).serialized_size(), 3);
        assert_eq!(VarInt(0x10000).serialized_size(), 5);
        assert_eq!(VarInt(0xFFFFFFFF).serialized_size(), 5);
        assert_eq!(VarInt(0x100000000).serialized_size(), 9);
    }

    #[test]
    fn test_varstring() {
        let varstring = VarString::new("Hello, Bitcoin!".to_string());
        assert_eq!(varstring.value(), "Hello, Bitcoin!");
        assert!(!varstring.is_empty());
        
        let empty = VarString::new(String::new());
        assert!(empty.is_empty());
    }

    #[test]
    fn test_serializer_bytes() {
        let mut serializer = Serializer::new();
        let data = b"Hello, World!";
        
        serializer.write_bytes(data);
        assert_eq!(serializer.data(), data);
        
        serializer.clear();
        assert!(serializer.is_empty());
    }

    #[test]
    fn test_deserializer_bounds_checking() {
        let data = vec![1, 2, 3];
        let mut deserializer = Deserializer::new(&data);
        
        assert_eq!(deserializer.remaining(), 3);
        assert_eq!(deserializer.read_u8().unwrap(), 1);
        assert_eq!(deserializer.remaining(), 2);
        
        // Try to read more than available
        assert!(deserializer.read_u32().is_err());
    }

    #[test]
    fn test_serializer_with_capacity() {
        let serializer = Serializer::with_capacity(100);
        assert_eq!(serializer.len(), 0);
        assert!(serializer.is_empty());
    }

    #[test]
    fn test_bool_serialization() {
        let mut serializer = Serializer::new();
        serializer.write_bool(true);
        serializer.write_bool(false);
        
        let data = serializer.data();
        let mut deserializer = Deserializer::new(data);
        
        assert_eq!(deserializer.read_bool().unwrap(), true);
        assert_eq!(deserializer.read_bool().unwrap(), false);
    }
}
