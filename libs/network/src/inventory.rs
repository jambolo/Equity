//! Inventory items for `inv`, `getdata`, and `notfound` messages.

use crate::serialize::{read_array, read_u32, write_bytes, write_u32};
use anyhow::{Result, bail};
use serde_json::json;

/// Length of an inventory hash.
pub const HASH_SIZE: usize = 32;
/// On-wire size of an `InventoryId`.
pub const INVENTORY_SIZE: usize = 4 + HASH_SIZE;

/// Tag identifying the kind of object referenced by an [`InventoryId`].
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InventoryType {
    /// Sentinel — invalid/unknown.
    Error = 0,
    /// Transaction (TXID).
    Transaction = 1,
    /// Block (block hash).
    Block = 2,
    /// Filtered (Merkle) block.
    FilteredBlock = 3,
}

impl TryFrom<u32> for InventoryType {
    type Error = anyhow::Error;

    fn try_from(v: u32) -> Result<Self> {
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

/// `inv`/`getdata`/`notfound` entry: a tagged 32-byte hash.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct InventoryId {
    /// Kind of object this id refers to.
    pub inv_type: InventoryType,
    /// Object hash (TXID for transactions, block hash for blocks).
    pub hash: [u8; HASH_SIZE],
}

impl InventoryId {
    /// Construct from a type tag and hash.
    pub fn new(inv_type: InventoryType, hash: [u8; HASH_SIZE]) -> Self {
        Self { inv_type, hash }
    }

    /// Shortcut for a transaction inventory entry.
    pub fn transaction(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::Transaction, hash)
    }

    /// Shortcut for a block inventory entry.
    pub fn block(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::Block, hash)
    }

    /// Shortcut for a filtered (Merkle) block inventory entry.
    pub fn filtered_block(hash: [u8; HASH_SIZE]) -> Self {
        Self::new(InventoryType::FilteredBlock, hash)
    }

    /// Append the wire encoding of this entry to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.inv_type as u32);
        write_bytes(out, &self.hash);
    }

    /// Parse from `stream`, advancing the cursor by [`INVENTORY_SIZE`].
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let inv_type = InventoryType::try_from(read_u32(stream)?)?;
        let hash: [u8; HASH_SIZE] = read_array(stream)?;
        Ok(Self { inv_type, hash })
    }

    /// JSON representation of this inventory entry.
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
