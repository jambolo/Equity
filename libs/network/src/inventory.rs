//! Inventory items for `inv`, `getdata`, and `notfound` messages.

use crate::serialize::{read_array, read_u32, write_bytes, write_u32};
use anyhow::{Result, bail};
use serde_json::json;

pub const HASH_SIZE: usize = 32;
pub const INVENTORY_SIZE: usize = 4 + HASH_SIZE;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InventoryType {
    Error = 0,
    Transaction = 1,
    Block = 2,
    FilteredBlock = 3,
}

impl InventoryType {
    pub fn from_u32(v: u32) -> Result<Self> {
        match v {
            0 => Ok(Self::Error),
            1 => Ok(Self::Transaction),
            2 => Ok(Self::Block),
            3 => Ok(Self::FilteredBlock),
            _ => bail!("unknown inventory type {}", v),
        }
    }
}

impl From<InventoryType> for u32 {
    fn from(t: InventoryType) -> Self {
        t as u32
    }
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct InventoryId {
    pub inv_type: InventoryType,
    pub hash: [u8; HASH_SIZE],
}

impl InventoryId {
    pub fn new(inv_type: InventoryType, hash: [u8; HASH_SIZE]) -> Self {
        Self { inv_type, hash }
    }

    pub fn transaction(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::Transaction, hash)
    }

    pub fn block(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::Block, hash)
    }

    pub fn filtered_block(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::FilteredBlock, hash)
    }

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.inv_type as u32);
        write_bytes(out, &self.hash);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let t = read_u32(stream)?;
        let inv_type = InventoryType::from_u32(t)?;
        let hash: [u8; HASH_SIZE] = read_array(stream)?;
        Ok(Self { inv_type, hash })
    }

    pub fn to_json(&self) -> serde_json::Value {
        json!({
            "type": self.inv_type as u32,
            "hash": hex::encode(self.hash),
        })
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn round_trip() {
        let inv = InventoryId::block([0xABu8; HASH_SIZE]);
        let mut bytes = Vec::new();
        inv.serialize(&mut bytes);
        assert_eq!(bytes.len(), INVENTORY_SIZE);
        let mut s = &bytes[..];
        let decoded = InventoryId::deserialize(&mut s).unwrap();
        assert_eq!(inv, decoded);
    }

    #[test]
    fn rejects_unknown_type() {
        let mut bytes = Vec::new();
        write_u32(&mut bytes, 99);
        bytes.extend_from_slice(&[0u8; HASH_SIZE]);
        assert!(InventoryId::deserialize(&mut &bytes[..]).is_err());
    }
}
