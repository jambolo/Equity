//! Account management

use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Account {
    pub id: String,
    pub balance: u64,
}

impl Account {
    pub fn new(id: String, balance: u64) -> Self {
        Self { id, balance }
    }
}
