//! Bitcoin block — header (80 bytes) + variable-length transaction list.

use crate::transaction::Transaction;
use crate::{EquityError, Result};
use p2p::{deserialize_var_int, serialize_var_int};

/// Length of a block/transaction hash in bytes.
pub const HASH_SIZE: usize = 32;

/// 80-byte Bitcoin block header.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct BlockHeader {
    /// Block version bits.
    pub version: i32,
    /// Hash of the previous block header (little-endian on wire).
    pub previous_block: [u8; HASH_SIZE],
    /// Merkle root of the block's transactions.
    pub merkle_root: [u8; HASH_SIZE],
    /// Unix timestamp from the miner.
    pub timestamp: u32,
    /// Compact-form difficulty target (`nBits`).
    pub target: u32,
    /// Mining nonce.
    pub nonce: u32,
}

/// Bitcoin block: 80-byte header + transaction list.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Block {
    header: BlockHeader,
    transactions: Vec<Transaction>,
}

impl BlockHeader {
    /// Parse a header from `stream`, advancing the cursor by 80 bytes.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let version = read_i32_le(stream)?;
        let previous_block = read_hash(stream)?;
        let merkle_root = read_hash(stream)?;
        let timestamp = read_u32_le(stream)?;
        let target = read_u32_le(stream)?;
        let nonce = read_u32_le(stream)?;
        Ok(Self {
            version,
            previous_block,
            merkle_root,
            timestamp,
            target,
            nonce,
        })
    }

    /// Append the 80-byte wire encoding of this header to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.version.to_le_bytes());
        out.extend_from_slice(&self.previous_block);
        out.extend_from_slice(&self.merkle_root);
        out.extend_from_slice(&self.timestamp.to_le_bytes());
        out.extend_from_slice(&self.target.to_le_bytes());
        out.extend_from_slice(&self.nonce.to_le_bytes());
    }
}

impl Block {
    /// Construct from a header and transaction list.
    pub fn new(header: BlockHeader, transactions: Vec<Transaction>) -> Self {
        Self { header, transactions }
    }

    /// Parse a full block from an owned byte buffer.
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let mut stream = data;
        Self::deserialize(&mut stream)
    }

    /// Parse a full block from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let header = BlockHeader::deserialize(stream)?;
        let n = read_var_int(stream)? as usize;
        let mut transactions = Vec::with_capacity(n);
        for _ in 0..n {
            transactions.push(Transaction::deserialize(stream)?);
        }
        Ok(Self { header, transactions })
    }

    /// On-wire bytes of this block (header + varint count + transactions).
    pub fn serialize(&self) -> Vec<u8> {
        let mut out = Vec::new();
        self.header.serialize(&mut out);
        out.extend_from_slice(&serialize_var_int(self.transactions.len() as u64));
        for tx in &self.transactions {
            out.extend_from_slice(&tx.serialize());
        }
        out
    }

    /// Borrowed block header.
    pub fn header(&self) -> &BlockHeader {
        &self.header
    }

    /// Borrowed transaction list.
    pub fn transactions(&self) -> &[Transaction] {
        &self.transactions
    }

    /// Number of transactions in the block.
    pub fn transaction_count(&self) -> usize {
        self.transactions.len()
    }

    /// Get transaction by index, or `None` if out of range.
    pub fn get_transaction(&self, i: usize) -> Option<&Transaction> {
        self.transactions.get(i)
    }
}

fn read_u32_le(stream: &mut &[u8]) -> Result<u32> {
    if stream.len() < 4 {
        return Err(EquityError("Truncated u32".to_string()));
    }
    let (head, rest) = stream.split_at(4);
    let v = u32::from_le_bytes([head[0], head[1], head[2], head[3]]);
    *stream = rest;
    Ok(v)
}

fn read_i32_le(stream: &mut &[u8]) -> Result<i32> {
    let u = read_u32_le(stream)?;
    Ok(u as i32)
}

fn read_hash(stream: &mut &[u8]) -> Result<[u8; HASH_SIZE]> {
    if stream.len() < HASH_SIZE {
        return Err(EquityError("Truncated hash".to_string()));
    }
    let (head, rest) = stream.split_at(HASH_SIZE);
    let mut out = [0u8; HASH_SIZE];
    out.copy_from_slice(head);
    *stream = rest;
    Ok(out)
}

fn read_var_int(stream: &mut &[u8]) -> Result<u64> {
    let r = deserialize_var_int(stream);
    if r.bytes_read == 0 {
        return Err(EquityError("Truncated varint".to_string()));
    }
    *stream = &stream[r.bytes_read..];
    Ok(r.value)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_header_round_trip() {
        let h = BlockHeader {
            version: 1,
            previous_block: [0u8; HASH_SIZE],
            merkle_root: [0xABu8; HASH_SIZE],
            timestamp: 1_700_000_000,
            target: 0x1d00ffff,
            nonce: 42,
        };
        let mut bytes = Vec::new();
        h.serialize(&mut bytes);
        assert_eq!(bytes.len(), 80);
        let mut s = &bytes[..];
        let parsed = BlockHeader::deserialize(&mut s).unwrap();
        assert_eq!(parsed, h);
    }

    #[test]
    fn test_block_empty_transactions_round_trip() {
        let block = Block::new(
            BlockHeader {
                version: 1,
                previous_block: [0u8; HASH_SIZE],
                merkle_root: [0u8; HASH_SIZE],
                timestamp: 0,
                target: 0x1d00ffff,
                nonce: 0,
            },
            vec![],
        );
        let bytes = block.serialize();
        let parsed = Block::from_data(&bytes).unwrap();
        assert_eq!(parsed, block);
    }

    #[test]
    fn test_truncated_block_rejected() {
        assert!(Block::from_data(&[0u8; 20]).is_err());
    }
}
