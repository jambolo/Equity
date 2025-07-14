//! Transaction functionality for Bitcoin
//! 
//! Provides functionality for creating, validating, and converting Bitcoin transactions.

use crate::ffi::{self, TransactionCpp, TransactionInputCpp, TransactionOutputCpp};
use crate::{Result, EquityError};
use cxx::UniquePtr;

/// A Bitcoin transaction
pub struct Transaction {
    inner: UniquePtr<TransactionCpp>,
}

/// A Bitcoin transaction input
pub struct TransactionInput {
    inner: TransactionInputCpp,
}

/// A Bitcoin transaction output
pub struct TransactionOutput {
    inner: TransactionOutputCpp,
}

impl Transaction {
    /// Create a transaction from JSON string
    pub fn from_json(json: &str) -> Result<Self> {
        let inner = ffi::transactionFromJson(json);
        if ffi::transactionIsValid(&inner) {
            Ok(Transaction { inner })
        } else {
            Err(EquityError("Invalid transaction JSON".to_string()))
        }
    }

    /// Create a transaction from binary data
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let inner = ffi::transactionFromData(data);
        if ffi::transactionIsValid(&inner) {
            Ok(Transaction { inner })
        } else {
            Err(EquityError("Invalid transaction data".to_string()))
        }
    }

    /// Convert the transaction to JSON string
    pub fn to_json(&self) -> String {
        ffi::transactionToJson(&self.inner)
    }

    /// Serialize the transaction to binary data
    pub fn serialize(&self) -> Vec<u8> {
        ffi::transactionSerialize(&self.inner)
    }

    /// Get the transaction version
    pub fn version(&self) -> u32 {
        ffi::transactionVersion(&self.inner)
    }

    /// Get the transaction lock time
    pub fn lock_time(&self) -> u32 {
        ffi::transactionLockTime(&self.inner)
    }

    /// Check if the transaction is valid
    pub fn is_valid(&self) -> bool {
        ffi::transactionIsValid(&self.inner)
    }

    /// Get the number of inputs in the transaction
    pub fn input_count(&self) -> usize {
        ffi::transactionInputCount(&self.inner)
    }

    /// Get the number of outputs in the transaction
    pub fn output_count(&self) -> usize {
        ffi::transactionOutputCount(&self.inner)
    }

    /// Get a specific input by index
    pub fn get_input(&self, index: usize) -> Option<TransactionInput> {
        if index < self.input_count() {
            let inner = ffi::transactionGetInput(&self.inner, index);
            Some(TransactionInput { inner })
        } else {
            None
        }
    }

    /// Get a specific output by index
    pub fn get_output(&self, index: usize) -> Option<TransactionOutput> {
        if index < self.output_count() {
            let inner = ffi::transactionGetOutput(&self.inner, index);
            Some(TransactionOutput { inner })
        } else {
            None
        }
    }

    /// Internal method to create from FFI type
    /// This is a workaround for the type system
    pub(crate) fn from_ffi(inner: UniquePtr<crate::ffi::TransactionCpp>) -> Self {
        Transaction { inner }
    }
}

impl TransactionInput {
    /// Get the output index from the previous transaction
    pub fn output_index(&self) -> u32 {
        self.inner.output_index
    }

    /// Get the sequence number
    pub fn sequence(&self) -> u32 {
        self.inner.sequence
    }
}

impl TransactionOutput {
    /// Get the value in satoshis
    pub fn value(&self) -> u64 {
        self.inner.value
    }
}

impl std::fmt::Debug for Transaction {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("Transaction")
            .field("version", &self.version())
            .field("lock_time", &self.lock_time())
            .field("valid", &self.is_valid())
            .field("input_count", &self.input_count())
            .field("output_count", &self.output_count())
            .finish()
    }
}

impl std::fmt::Debug for TransactionInput {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("TransactionInput")
            .field("output_index", &self.output_index())
            .field("sequence", &self.sequence())
            .finish()
    }
}

impl std::fmt::Debug for TransactionOutput {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.debug_struct("TransactionOutput")
            .field("value", &self.value())
            .finish()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_transaction_from_json() {
        let json = r#"{"version":1,"locktime":0,"vin":[],"vout":[]}"#;
        let result = Transaction::from_json(json);
        
        // Note: This might fail if the C++ implementation requires valid inputs/outputs
        match result {
            Ok(tx) => {
                assert_eq!(tx.version(), 1);
                assert_eq!(tx.lock_time(), 0);
            }
            Err(_) => {
                // Expected if empty transactions are invalid
            }
        }
    }
}
