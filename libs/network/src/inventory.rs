//! Network inventory utilities

use crate::ffi;

/// Inventory type identifiers
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum InventoryType {
    Error = 0,         // Any data with this number may be ignored
    Transaction = 1,   // Hash is related to a transaction
    Block = 2,         // Hash is related to a data block
    FilteredBlock = 3, // Hash of a block header, indicates reply should be merkle block
}

impl From<u32> for InventoryType {
    fn from(value: u32) -> Self {
        match value {
            0 => InventoryType::Error,
            1 => InventoryType::Transaction,
            2 => InventoryType::Block,
            3 => InventoryType::FilteredBlock,
            _ => InventoryType::Error,
        }
    }
}

impl From<InventoryType> for u32 {
    fn from(inv_type: InventoryType) -> Self {
        inv_type as u32
    }
}

/// Inventory item representing an object in inventory messages
#[derive(Debug, Clone)]
pub struct InventoryId {
    pub inv_type: InventoryType,
    pub hash: Vec<u8>, // 32 bytes SHA256 hash
}

impl InventoryId {
    /// Create a new inventory ID
    pub fn new(inv_type: InventoryType, hash: Vec<u8>) -> Result<Self, &'static str> {
        if hash.len() != 32 {
            return Err("Hash must be exactly 32 bytes (SHA256)");
        }
        
        Ok(InventoryId { inv_type, hash })
    }
    
    /// Create transaction inventory
    pub fn transaction(hash: Vec<u8>) -> Result<Self, &'static str> {
        Self::new(InventoryType::Transaction, hash)
    }
    
    /// Create block inventory
    pub fn block(hash: Vec<u8>) -> Result<Self, &'static str> {
        Self::new(InventoryType::Block, hash)
    }
    
    /// Create filtered block inventory
    pub fn filtered_block(hash: Vec<u8>) -> Result<Self, &'static str> {
        Self::new(InventoryType::FilteredBlock, hash)
    }
    
    /// Serialize the inventory ID to bytes
    pub fn serialize(&self) -> Result<Vec<u8>, &'static str> {
        let mut output = Vec::new();
        let success = ffi::networkInventorySerialize(self.inv_type.into(), &self.hash, &mut output);
        
        if success {
            Ok(output)
        } else {
            Err("Failed to serialize inventory")
        }
    }
    
    /// Deserialize inventory ID from bytes
    pub fn deserialize(data: &[u8]) -> Result<Self, &'static str> {
        let data_vec = data.to_vec();
        let mut type_id = 0u32;
        let mut hash = Vec::new();
        
        let success = ffi::networkInventoryDeserialize(&data_vec, &mut type_id, &mut hash);
        
        if success {
            Ok(InventoryId {
                inv_type: InventoryType::from(type_id),
                hash,
            })
        } else {
            Err("Failed to deserialize inventory")
        }
    }
    
    /// Convert to JSON string
    pub fn to_json(&self) -> Result<String, &'static str> {
        let mut json = String::new();
        let success = ffi::networkInventoryToJson(self.inv_type.into(), &self.hash, &mut json);
        
        if success {
            Ok(json)
        } else {
            Err("Failed to convert inventory to JSON")
        }
    }
}

// Convenience functions
pub fn create_inventory(inv_type: InventoryType, hash: &[u8; 32]) -> Result<Vec<u8>, &'static str> {
    let hash_vec = hash.to_vec();
    let mut serialized = Vec::new();
    let success = ffi::networkInventoryCreate(inv_type.into(), &hash_vec, &mut serialized);
    
    if success {
        Ok(serialized)
    } else {
        Err("Failed to create inventory")
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_inventory_type_conversion() {
        assert_eq!(InventoryType::from(0), InventoryType::Error);
        assert_eq!(InventoryType::from(1), InventoryType::Transaction);
        assert_eq!(InventoryType::from(2), InventoryType::Block);
        assert_eq!(InventoryType::from(3), InventoryType::FilteredBlock);
        assert_eq!(InventoryType::from(999), InventoryType::Error); // Unknown becomes Error
        
        assert_eq!(u32::from(InventoryType::Error), 0);
        assert_eq!(u32::from(InventoryType::Transaction), 1);
        assert_eq!(u32::from(InventoryType::Block), 2);
        assert_eq!(u32::from(InventoryType::FilteredBlock), 3);
    }
    
    #[test]
    fn test_inventory_creation() {
        let hash = vec![0u8; 32];
        let inv = InventoryId::new(InventoryType::Transaction, hash.clone());
        assert!(inv.is_ok());
        
        let inv = inv.unwrap();
        assert_eq!(inv.inv_type, InventoryType::Transaction);
        assert_eq!(inv.hash, hash);
        
        // Test invalid hash size
        let invalid_hash = vec![0u8; 20]; // Wrong size
        let inv = InventoryId::new(InventoryType::Transaction, invalid_hash);
        assert!(inv.is_err());
    }
    
    #[test]
    fn test_inventory_convenience_constructors() {
        let hash = vec![1u8; 32];
        
        let tx_inv = InventoryId::transaction(hash.clone()).unwrap();
        assert_eq!(tx_inv.inv_type, InventoryType::Transaction);
        
        let block_inv = InventoryId::block(hash.clone()).unwrap();
        assert_eq!(block_inv.inv_type, InventoryType::Block);
        
        let filtered_inv = InventoryId::filtered_block(hash.clone()).unwrap();
        assert_eq!(filtered_inv.inv_type, InventoryType::FilteredBlock);
    }
    
    #[test]
    fn test_inventory_serialization() {
        let hash = vec![0u8; 32];
        let inv = InventoryId::new(InventoryType::Transaction, hash).unwrap();
        
        // Test serialization
        if let Ok(serialized) = inv.serialize() {
            assert!(!serialized.is_empty());
            
            // Test deserialization
            if let Ok(deserialized) = InventoryId::deserialize(&serialized) {
                assert_eq!(inv.inv_type, deserialized.inv_type);
                assert_eq!(inv.hash, deserialized.hash);
            }
        }
    }
}
