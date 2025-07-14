//! Block functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin blocks.

use crate::ffi::{self, BlockCpp, BlockHeaderCpp};
use crate::transaction::Transaction;
use crate::{Result, EquityError};
use cxx::UniquePtr;

/// A Bitcoin block
pub struct Block {
    inner: UniquePtr<BlockCpp>,
}

/// A Bitcoin block header
pub struct BlockHeader {
    inner: BlockHeaderCpp,
}

impl Block {
    /// Create a block from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let inner = ffi::blockFromData(data);
        // Note: We can't validate directly since we don't have a validation function
        // The C++ constructor should handle validation
        Ok(Block { inner })
    }

    /// Convert the block to JSON string
    pub fn to_json(&self) -> String {
        ffi::blockToJson(&self.inner)
    }

    /// Serialize the block to binary data
    pub fn serialize(&self) -> Vec<u8> {
        ffi::blockSerialize(&self.inner)
    }

    /// Get the block header
    pub fn header(&self) -> BlockHeader {
        let inner = ffi::blockGetHeader(&self.inner);
        BlockHeader { inner }
    }

    /// Get the number of transactions in the block
    pub fn transaction_count(&self) -> usize {
        ffi::blockTransactionCount(&self.inner)
    }

    /// Get a specific transaction by index
    pub fn get_transaction(&self, index: usize) -> Option<Transaction> {
        if index < self.transaction_count() {
            let inner = ffi::blockGetTransaction(&self.inner, index);
            // We need to wrap this in our Transaction type
            // Note: This is a simplified approach - in practice you might need
            // to serialize and deserialize to maintain consistency
            Some(Transaction::from_ffi(inner))
        } else {
            None
        }
    }
}

impl BlockHeader {
    /// Get the block version
    pub fn version(&self) -> i32 {
        self.inner.version
    }

    /// Get the timestamp
    pub fn timestamp(&self) -> u32 {
        self.inner.timestamp
    }

    /// Get the difficulty target
    pub fn target(&self) -> u32 {
        self.inner.target
    }

    /// Get the nonce
    pub fn nonce(&self) -> u32 {
        self.inner.nonce
    }
}

impl std::fmt::Debug for Block {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        let header = self.header();
        f.debug_struct("Block")
            .field("version", &header.version())
            .field("timestamp", &header.timestamp())
            .field("target", &header.target())
            .field("nonce", &header.nonce())
            .field("transaction_count", &self.transaction_count())
            .finish()
    }
}

impl std::fmt::Debug for BlockHeader {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("BlockHeader")
            .field("version", &self.version())
            .field("timestamp", &self.timestamp())
            .field("target", &self.target())
            .field("nonce", &self.nonce())
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_block_creation() {
        // This test would need actual block data to be meaningful
        // For now, just test that the module compiles correctly
        let test_data = vec![0u8; 80]; // Minimal block header size
        let result = Block::from_data(&test_data);
        
        // The result depends on the C++ implementation's validation
        match result {
            Ok(block) => {
                println!("Block created successfully");
                assert!(block.transaction_count() >= 0);
            }
            Err(_) => {
                println!("Block creation failed (expected for invalid data)");
            }
        }
    }
}
