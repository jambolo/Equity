//! Transaction types and utilities

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Transaction {
    pub id: String,
    pub from: String,
    pub to: String,
    pub amount: u64,
    pub timestamp: u64,
}

impl Transaction {
    pub fn new(id: String, from: String, to: String, amount: u64) -> Self {
        Self {
            id,
            from,
            to,
            amount,
            timestamp: 0, // TODO: Use actual timestamp
        }
    }
}
