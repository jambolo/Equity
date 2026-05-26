//! Bitcoin script — a parsed sequence of `Instruction`s plus the raw bytes.

use crate::instruction::{Instruction, DESCRIPTIONS, ScriptParsingError};
use crate::{EquityError, Result};

/// Parsed Bitcoin script — raw bytes plus the decoded instruction list.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Script {
    data: Vec<u8>,
    instructions: Vec<Instruction>,
    valid: bool,
}

impl Script {
    /// Build from raw bytes; instructions are parsed eagerly.
    pub fn from_data(data: &[u8]) -> Result<Self> {
        let mut s = Self {
            data: data.to_vec(),
            instructions: Vec::new(),
            valid: false,
        };
        s.valid = s.parse().is_ok();
        Ok(s)
    }

    /// Build from a hex-encoded script.
    pub fn from_hex(s: &str) -> Result<Self> {
        let bytes = hex::decode(s).map_err(|e| EquityError(format!("Invalid hex: {e}")))?;
        Self::from_data(&bytes)
    }

    /// Raw script bytes.
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Decoded instructions.
    pub fn instructions(&self) -> &[Instruction] {
        &self.instructions
    }

    /// True if parsing succeeded.
    pub fn is_valid(&self) -> bool {
        self.valid
    }

    /// True if the script has zero bytes.
    pub fn is_empty(&self) -> bool {
        self.data.is_empty()
    }

    /// Length of the raw script bytes.
    pub fn len(&self) -> usize {
        self.data.len()
    }

    /// Append the raw script bytes to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.data);
    }

    /// Lower-case hex of the raw script bytes.
    pub fn to_hex(&self) -> String {
        hex::encode(&self.data)
    }

    /// Render as human-readable script source (opcode names and push hex).
    pub fn to_source(&self) -> String {
        let mut parts = Vec::new();
        for ins in &self.instructions {
            if (0x01..=0x4f).contains(&ins.op()) {
                parts.push(hex::encode(ins.data()));
            } else {
                parts.push(DESCRIPTIONS[ins.op() as usize].name.to_string());
            }
        }
        parts.join(" ")
    }

    fn parse(&mut self) -> std::result::Result<(), ScriptParsingError> {
        self.instructions.clear();
        let mut cursor = &self.data[..];
        let start = self.data.as_ptr();
        while !cursor.is_empty() {
            let location = unsafe { cursor.as_ptr().offset_from(start) } as usize;
            let ins = Instruction::parse(&mut cursor, location)?;
            self.instructions.push(ins);
        }
        Ok(())
    }
}

impl std::fmt::Display for Script {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.to_hex())
    }
}

impl std::str::FromStr for Script {
    type Err = EquityError;

    fn from_str(s: &str) -> Result<Self> {
        Self::from_hex(s)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_script_is_valid_and_empty() {
        let s = Script::from_data(&[]).unwrap();
        assert!(s.is_valid());
        assert!(s.is_empty());
        assert!(s.instructions().is_empty());
    }

    #[test]
    fn test_p2pkh_script_round_trip() {
        // OP_DUP OP_HASH160 PUSH(20) <20 bytes> OP_EQUALVERIFY OP_CHECKSIG
        let mut bytes = vec![0x76, 0xa9, 0x14];
        bytes.extend_from_slice(&[0u8; 20]);
        bytes.extend_from_slice(&[0x88, 0xac]);
        let s = Script::from_data(&bytes).unwrap();
        assert!(s.is_valid());
        assert_eq!(s.instructions().len(), 5);
        assert_eq!(s.instructions()[0].op(), 0x76);
        assert_eq!(s.instructions()[2].data().len(), 20);
        assert_eq!(s.to_hex().len(), bytes.len() * 2);
    }

    #[test]
    fn test_invalid_op_marks_script_invalid() {
        let bytes = [0xBA]; // unassigned opcode
        let s = Script::from_data(&bytes).unwrap();
        assert!(!s.is_valid());
    }

    #[test]
    fn test_hex_round_trip() {
        let bytes = [0x51u8, 0x52, 0x93]; // OP_1 OP_2 OP_ADD
        let s = Script::from_hex(&hex::encode(bytes)).unwrap();
        assert_eq!(s.data(), bytes);
        assert!(s.is_valid());
    }
}
