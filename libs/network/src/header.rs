//! Bitcoin P2P message header (24 bytes).
//!
//! ```text
//! magic:    u32 little-endian   ( 4 bytes)
//! command:  ASCII null-padded   (12 bytes)
//! length:   u32 little-endian   ( 4 bytes)
//! checksum: first 4 bytes of SHA256(SHA256(payload))  (4 bytes)
//! ```

use crate::serialize::{read_array, read_u32, write_bytes, write_u32};
use anyhow::{Result, bail};
use crypto::sha256;

/// Mainnet magic.
pub const MAGIC_MAIN: u32 = 0xD9B4BEF9;
/// Testnet magic.
pub const MAGIC_TEST: u32 = 0xDAB5BFFA;
/// Testnet3 magic.
pub const MAGIC_TEST3: u32 = 0x0709110B;

/// Length of the null-padded `command` field.
pub const COMMAND_SIZE: usize = 12;
/// Total on-wire header size in bytes.
pub const HEADER_SIZE: usize = 24;

/// Bitcoin P2P message header.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Header {
    /// Network magic (e.g. [`MAGIC_MAIN`]).
    pub magic: u32,
    /// ASCII command name, null-padded to [`COMMAND_SIZE`].
    pub command: [u8; COMMAND_SIZE],
    /// Length of the payload that follows.
    pub length: u32,
    /// First 4 bytes of `double_sha256(payload)`, little-endian.
    pub checksum: u32,
}

impl Header {
    /// Build a header for `command` over `payload` with the given `magic`.
    ///
    /// Errors if `command` is `COMMAND_SIZE` bytes or longer (no room for null terminator).
    pub fn new(magic: u32, command: &str, payload: &[u8]) -> Result<Self> {
        if command.len() >= COMMAND_SIZE {
            bail!("command '{}' exceeds {} bytes", command, COMMAND_SIZE - 1);
        }
        let mut command_bytes = [0u8; COMMAND_SIZE];
        command_bytes[..command.len()].copy_from_slice(command.as_bytes());
        Ok(Self {
            magic,
            command: command_bytes,
            length: payload.len() as u32,
            checksum: payload_checksum(payload),
        })
    }

    /// Decode the command field as a UTF-8 string up to the first null byte.
    pub fn command_str(&self) -> &str {
        let end = self.command.iter().position(|&b| b == 0).unwrap_or(COMMAND_SIZE);
        std::str::from_utf8(&self.command[..end]).unwrap_or("")
    }

    /// Append the 24-byte on-wire encoding of this header to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.magic);
        write_bytes(out, &self.command);
        write_u32(out, self.length);
        write_u32(out, self.checksum);
    }

    /// On-wire bytes as a fresh `Vec`.
    pub fn to_bytes(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(HEADER_SIZE);
        self.serialize(&mut out);
        out
    }

    /// Parse a header from `stream` and advance the cursor by [`HEADER_SIZE`].
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let magic = read_u32(stream)?;
        let command: [u8; COMMAND_SIZE] = read_array(stream)?;
        let length = read_u32(stream)?;
        let checksum = read_u32(stream)?;
        Ok(Self {
            magic,
            command,
            length,
            checksum,
        })
    }
}

/// Compute the 4-byte little-endian Bitcoin checksum of `payload`.
pub fn payload_checksum(payload: &[u8]) -> u32 {
    let bytes = sha256::checksum(payload);
    u32::from_le_bytes(bytes)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn round_trip() {
        let h = Header::new(MAGIC_MAIN, "verack", &[]).unwrap();
        let bytes = h.to_bytes();
        assert_eq!(bytes.len(), HEADER_SIZE);
        let mut s = &bytes[..];
        let parsed = Header::deserialize(&mut s).unwrap();
        assert_eq!(h, parsed);
        assert_eq!(parsed.command_str(), "verack");
    }

    #[test]
    fn empty_payload_checksum() {
        let c = payload_checksum(&[]);
        assert_eq!(c, 0xe2e0f65d);
    }

    #[test]
    fn rejects_overlong_command() {
        assert!(Header::new(MAGIC_MAIN, "this_is_too_long", &[]).is_err());
    }
}
