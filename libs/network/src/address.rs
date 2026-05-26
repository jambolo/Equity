//! Bitcoin network address (30 bytes on the wire).
//!
//! Wire layout (matching the legacy C++ implementation):
//! ```text
//! time:     u32 little-endian   ( 4 bytes)
//! services: u64 little-endian   ( 8 bytes)
//! ipv6:     [u8; 16] raw        (16 bytes)
//! port:     u16 little-endian   ( 2 bytes)
//! ```
//! The port is stored little-endian for parity with the existing C++ code,
//! even though strict Bitcoin protocol specifies big-endian.

use crate::serialize::{
    read_array, read_u16, read_u32, read_u64, write_bytes, write_u16, write_u32, write_u64,
};
use anyhow::Result;
use serde_json::json;

pub const ADDRESS_SIZE: usize = 30;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Address {
    pub time: u32,
    pub services: u64,
    pub ipv6: [u8; 16],
    pub port: u16,
}

impl Address {
    pub fn new(time: u32, services: u64, ipv6: [u8; 16], port: u16) -> Self {
        Self {
            time,
            services,
            ipv6,
            port,
        }
    }

    pub fn from_ipv4(time: u32, services: u64, ipv4: [u8; 4], port: u16) -> Self {
        let mut ipv6 = [0u8; 16];
        ipv6[10] = 0xff;
        ipv6[11] = 0xff;
        ipv6[12..16].copy_from_slice(&ipv4);
        Self::new(time, services, ipv6, port)
    }

    pub fn ipv4(&self) -> Option<[u8; 4]> {
        if self.ipv6[10] == 0xff
            && self.ipv6[11] == 0xff
            && self.ipv6[..10].iter().all(|&b| b == 0)
        {
            let mut out = [0u8; 4];
            out.copy_from_slice(&self.ipv6[12..16]);
            Some(out)
        } else {
            None
        }
    }

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.time);
        write_u64(out, self.services);
        write_bytes(out, &self.ipv6);
        write_u16(out, self.port);
    }

    pub fn to_bytes(&self) -> Vec<u8> {
        let mut out = Vec::with_capacity(ADDRESS_SIZE);
        self.serialize(&mut out);
        out
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let time = read_u32(stream)?;
        let services = read_u64(stream)?;
        let ipv6: [u8; 16] = read_array(stream)?;
        let port = read_u16(stream)?;
        Ok(Self {
            time,
            services,
            ipv6,
            port,
        })
    }

    pub fn from_bytes(data: &[u8]) -> Result<Self> {
        let mut s = data;
        Self::deserialize(&mut s)
    }

    pub fn to_json(&self) -> serde_json::Value {
        json!({
            "time": self.time,
            "services": self.services,
            "ipv6": hex::encode(self.ipv6),
            "port": self.port,
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn round_trip() {
        let a = Address::from_ipv4(1_700_000_000, 1, [192, 168, 0, 1], 8333);
        let bytes = a.to_bytes();
        assert_eq!(bytes.len(), ADDRESS_SIZE);
        let b = Address::from_bytes(&bytes).unwrap();
        assert_eq!(a, b);
    }

    #[test]
    fn ipv4_recovery() {
        let a = Address::from_ipv4(0, 0, [10, 0, 0, 1], 18333);
        assert_eq!(a.ipv4(), Some([10, 0, 0, 1]));
    }

    #[test]
    fn non_v4_returns_none() {
        let mut ipv6 = [0u8; 16];
        ipv6[0] = 0x20;
        let a = Address::new(0, 0, ipv6, 0);
        assert_eq!(a.ipv4(), None);
    }
}
