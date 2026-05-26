//! Bitcoin script instruction: opcode byte + optional push payload.
//!
//! A flat 256-entry opcode table drives
//! parsing, serialization, and the engine's `min_args` check.

/// One Bitcoin script instruction: opcode byte plus any push payload.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Instruction {
    op: u8,
    data: Vec<u8>,
    location: usize,
    size: usize,
    valid: bool,
}

/// Returned when [`Instruction::parse`] encounters malformed bytes.
#[derive(Debug)]
pub struct ScriptParsingError;

impl std::fmt::Display for ScriptParsingError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "script parsing error")
    }
}

impl std::error::Error for ScriptParsingError {}

/// Named Bitcoin script opcodes. Numeric value equals the on-wire byte.
#[allow(non_camel_case_types)]
#[repr(u8)]
#[derive(Debug, Clone, Copy)]
pub enum OpCode {
    OP_0 = 0x00,
    OP_PUSHDATA1 = 0x4c,
    OP_PUSHDATA2 = 0x4d,
    OP_PUSHDATA4 = 0x4e,
    OP_1NEGATE = 0x4f,
    OP_RESERVED = 0x50,
    OP_1 = 0x51,
    OP_16 = 0x60,
    OP_NOP = 0x61,
    OP_VER = 0x62,
    OP_IF = 0x63,
    OP_NOTIF = 0x64,
    OP_VERIF = 0x65,
    OP_VERNOTIF = 0x66,
    OP_ELSE = 0x67,
    OP_ENDIF = 0x68,
    OP_VERIFY = 0x69,
    OP_RETURN = 0x6a,
    OP_TOALTSTACK = 0x6b,
    OP_FROMALTSTACK = 0x6c,
    OP_2DROP = 0x6d,
    OP_2DUP = 0x6e,
    OP_3DUP = 0x6f,
    OP_2OVER = 0x70,
    OP_2ROT = 0x71,
    OP_2SWAP = 0x72,
    OP_IFDUP = 0x73,
    OP_DEPTH = 0x74,
    OP_DROP = 0x75,
    OP_DUP = 0x76,
    OP_NIP = 0x77,
    OP_OVER = 0x78,
    OP_PICK = 0x79,
    OP_ROLL = 0x7a,
    OP_ROT = 0x7b,
    OP_SWAP = 0x7c,
    OP_TUCK = 0x7d,
    OP_SIZE = 0x82,
    OP_EQUAL = 0x87,
    OP_EQUALVERIFY = 0x88,
    OP_RESERVED1 = 0x89,
    OP_RESERVED2 = 0x8a,
    OP_1ADD = 0x8b,
    OP_1SUB = 0x8c,
    OP_NEGATE = 0x8f,
    OP_ABS = 0x90,
    OP_NOT = 0x91,
    OP_0NOTEQUAL = 0x92,
    OP_ADD = 0x93,
    OP_SUB = 0x94,
    OP_BOOLAND = 0x9a,
    OP_BOOLOR = 0x9b,
    OP_NUMEQUAL = 0x9c,
    OP_NUMEQUALVERIFY = 0x9d,
    OP_NUMNOTEQUAL = 0x9e,
    OP_LESSTHAN = 0x9f,
    OP_GREATERTHAN = 0xa0,
    OP_LESSTHANOREQUAL = 0xa1,
    OP_GREATERTHANOREQUAL = 0xa2,
    OP_MIN = 0xa3,
    OP_MAX = 0xa4,
    OP_WITHIN = 0xa5,
    OP_RIPEMD160 = 0xa6,
    OP_SHA1 = 0xa7,
    OP_SHA256 = 0xa8,
    OP_HASH160 = 0xa9,
    OP_HASH256 = 0xaa,
    OP_CODESEPARATOR = 0xab,
    OP_CHECKSIG = 0xac,
    OP_CHECKSIGVERIFY = 0xad,
    OP_CHECKMULTISIG = 0xae,
    OP_CHECKMULTISIGVERIFY = 0xaf,
    OP_NOP1 = 0xb0,
    OP_CHECKLOCKTIMEVERIFY = 0xb1,
    OP_NOP3 = 0xb2,
    OP_NOP10 = 0xb9,
    OP_INVALID = 0xff,
}

/// Static metadata for one opcode slot.
#[derive(Debug, Clone, Copy)]
pub struct Description {
    /// Display name (e.g. `"DUP"`).
    pub name: &'static str,
    /// Minimum stack depth the engine requires before executing this opcode.
    pub min_args: usize,
    /// True if this byte is a defined opcode.
    pub valid: bool,
}

const fn d(name: &'static str, min_args: usize, valid: bool) -> Description {
    Description {
        name,
        min_args,
        valid,
    }
}

/// 256-entry opcode descriptor table.
pub static DESCRIPTIONS: [Description; 256] = build_descriptions();

const fn build_descriptions() -> [Description; 256] {
    let mut t = [d("(invalid)", 0, false); 256];
    t[0x00] = d("0", 0, true);
    // 0x01..=0x4b push N bytes
    let mut i = 0x01usize;
    while i <= 0x4b {
        t[i] = d("PUSHDATA", 0, true);
        i += 1;
    }
    t[0x4c] = d("PUSHDATA1", 0, true);
    t[0x4d] = d("PUSHDATA2", 0, true);
    t[0x4e] = d("PUSHDATA4", 0, true);
    t[0x4f] = d("1NEGATE", 0, true);
    t[0x50] = d("RESERVED", 0, true);
    // 0x51..=0x60 push 1..=16
    let mut n = 0x51usize;
    while n <= 0x60 {
        t[n] = match n {
            0x51 => d("1", 0, true),
            0x52 => d("2", 0, true),
            0x53 => d("3", 0, true),
            0x54 => d("4", 0, true),
            0x55 => d("5", 0, true),
            0x56 => d("6", 0, true),
            0x57 => d("7", 0, true),
            0x58 => d("8", 0, true),
            0x59 => d("9", 0, true),
            0x5a => d("10", 0, true),
            0x5b => d("11", 0, true),
            0x5c => d("12", 0, true),
            0x5d => d("13", 0, true),
            0x5e => d("14", 0, true),
            0x5f => d("15", 0, true),
            0x60 => d("16", 0, true),
            _ => d("(invalid)", 0, false),
        };
        n += 1;
    }
    t[0x61] = d("NOP", 0, true);
    t[0x62] = d("VER", 0, true);
    t[0x63] = d("IF", 1, true);
    t[0x64] = d("NOTIF", 1, true);
    t[0x65] = d("VERIF", 0, false);
    t[0x66] = d("VERNOTIF", 0, false);
    t[0x67] = d("ELSE", 0, true);
    t[0x68] = d("ENDIF", 0, true);
    t[0x69] = d("VERIFY", 1, true);
    t[0x6a] = d("RETURN", 0, true);
    t[0x6b] = d("TOALTSTACK", 1, true);
    t[0x6c] = d("FROMALTSTACK", 0, true);
    t[0x6d] = d("2DROP", 2, true);
    t[0x6e] = d("2DUP", 2, true);
    t[0x6f] = d("3DUP", 3, true);
    t[0x70] = d("2OVER", 4, true);
    t[0x71] = d("2ROT", 6, true);
    t[0x72] = d("2SWAP", 4, true);
    t[0x73] = d("IFDUP", 1, true);
    t[0x74] = d("DEPTH", 0, true);
    t[0x75] = d("DROP", 1, true);
    t[0x76] = d("DUP", 1, true);
    t[0x77] = d("NIP", 2, true);
    t[0x78] = d("OVER", 2, true);
    t[0x79] = d("PICK", 2, true);
    t[0x7a] = d("ROLL", 2, true);
    t[0x7b] = d("ROT", 3, true);
    t[0x7c] = d("SWAP", 2, true);
    t[0x7d] = d("TUCK", 2, true);
    t[0x82] = d("SIZE", 1, true);
    t[0x87] = d("EQUAL", 2, true);
    t[0x88] = d("EQUALVERIFY", 2, true);
    t[0x89] = d("RESERVED1", 0, true);
    t[0x8a] = d("RESERVED2", 0, true);
    t[0x8b] = d("1ADD", 1, true);
    t[0x8c] = d("1SUB", 1, true);
    t[0x8f] = d("NEGATE", 1, true);
    t[0x90] = d("ABS", 1, true);
    t[0x91] = d("NOT", 1, true);
    t[0x92] = d("0NOTEQUAL", 1, true);
    t[0x93] = d("ADD", 2, true);
    t[0x94] = d("SUB", 2, true);
    t[0x9a] = d("BOOLAND", 2, true);
    t[0x9b] = d("BOOLOR", 2, true);
    t[0x9c] = d("NUMEQUAL", 2, true);
    t[0x9d] = d("NUMEQUALVERIFY", 2, true);
    t[0x9e] = d("NUMNOTEQUAL", 2, true);
    t[0x9f] = d("LESSTHAN", 2, true);
    t[0xa0] = d("GREATERTHAN", 2, true);
    t[0xa1] = d("LESSTHANOREQUAL", 2, true);
    t[0xa2] = d("GREATERTHANOREQUAL", 2, true);
    t[0xa3] = d("MIN", 2, true);
    t[0xa4] = d("MAX", 2, true);
    t[0xa5] = d("WITHIN", 3, true);
    t[0xa6] = d("RIPEMD160", 1, true);
    t[0xa7] = d("SHA1", 1, true);
    t[0xa8] = d("SHA256", 1, true);
    t[0xa9] = d("HASH160", 1, true);
    t[0xaa] = d("HASH256", 1, true);
    t[0xab] = d("CODESEPARATOR", 0, true);
    t[0xac] = d("CHECKSIG", 2, true);
    t[0xad] = d("CHECKSIGVERIFY", 2, true);
    t[0xae] = d("CHECKMULTISIG", 3, true);
    t[0xaf] = d("CHECKMULTISIGVERIFY", 3, true);
    t[0xb0] = d("NOP1", 0, true);
    t[0xb1] = d("CHECKLOCKTIMEVERIFY", 0, true);
    let mut k = 0xb2usize;
    while k <= 0xb9 {
        let name = match k {
            0xb2 => "NOP3",
            0xb3 => "NOP4",
            0xb4 => "NOP5",
            0xb5 => "NOP6",
            0xb6 => "NOP7",
            0xb7 => "NOP8",
            0xb8 => "NOP9",
            0xb9 => "NOP10",
            _ => "(invalid)",
        };
        t[k] = d(name, 0, true);
        k += 1;
    }
    t
}

impl Instruction {
    /// Opcode byte.
    pub fn op(&self) -> u8 {
        self.op
    }

    /// Push payload (empty for non-push opcodes).
    pub fn data(&self) -> &[u8] {
        &self.data
    }

    /// Byte offset of this instruction within its source script.
    pub fn location(&self) -> usize {
        self.location
    }

    /// Total encoded size in bytes (opcode + length prefix + payload).
    pub fn size(&self) -> usize {
        self.size
    }

    /// True if this instruction was successfully parsed.
    pub fn is_valid(&self) -> bool {
        self.valid
    }

    /// Static metadata for this instruction's opcode.
    pub fn description(&self) -> Description {
        DESCRIPTIONS[self.op as usize]
    }

    /// Parse one instruction from the byte stream. Advances `cursor` (a mutable
    /// slice view) and records the offset of this instruction within `start`.
    pub fn parse(stream: &mut &[u8], location: usize) -> Result<Self, ScriptParsingError> {
        let op = *stream.first().ok_or(ScriptParsingError)?;
        *stream = &stream[1..];
        let mut size = 1usize;

        let desc = DESCRIPTIONS[op as usize];
        if !desc.valid {
            return Err(ScriptParsingError);
        }

        let data = if (0x01..=0x4b).contains(&op) {
            let count = op as usize;
            if stream.len() < count {
                return Err(ScriptParsingError);
            }
            let bytes = stream[..count].to_vec();
            *stream = &stream[count..];
            size += count;
            bytes
        } else if op == OpCode::OP_PUSHDATA1 as u8 {
            if stream.is_empty() {
                return Err(ScriptParsingError);
            }
            let count = stream[0] as usize;
            *stream = &stream[1..];
            size += 1;
            if stream.len() < count {
                return Err(ScriptParsingError);
            }
            let bytes = stream[..count].to_vec();
            *stream = &stream[count..];
            size += count;
            bytes
        } else if op == OpCode::OP_PUSHDATA2 as u8 {
            if stream.len() < 2 {
                return Err(ScriptParsingError);
            }
            let count = u16::from_le_bytes([stream[0], stream[1]]) as usize;
            *stream = &stream[2..];
            size += 2;
            if stream.len() < count {
                return Err(ScriptParsingError);
            }
            let bytes = stream[..count].to_vec();
            *stream = &stream[count..];
            size += count;
            bytes
        } else if op == OpCode::OP_PUSHDATA4 as u8 {
            if stream.len() < 4 {
                return Err(ScriptParsingError);
            }
            let count = u32::from_le_bytes([stream[0], stream[1], stream[2], stream[3]]) as usize;
            *stream = &stream[4..];
            size += 4;
            if stream.len() < count {
                return Err(ScriptParsingError);
            }
            let bytes = stream[..count].to_vec();
            *stream = &stream[count..];
            size += count;
            bytes
        } else {
            Vec::new()
        };

        Ok(Self {
            op,
            data,
            location,
            size,
            valid: true,
        })
    }

    /// Append the on-wire encoding of this instruction to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        if !self.valid {
            out.push(OpCode::OP_INVALID as u8);
            return;
        }
        out.push(self.op);
        if (0x01..=0x4b).contains(&self.op) {
            out.extend_from_slice(&self.data);
        } else if self.op == OpCode::OP_PUSHDATA1 as u8 {
            out.push(self.data.len() as u8);
            out.extend_from_slice(&self.data);
        } else if self.op == OpCode::OP_PUSHDATA2 as u8 {
            out.extend_from_slice(&(self.data.len() as u16).to_le_bytes());
            out.extend_from_slice(&self.data);
        } else if self.op == OpCode::OP_PUSHDATA4 as u8 {
            out.extend_from_slice(&(self.data.len() as u32).to_le_bytes());
            out.extend_from_slice(&self.data);
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_pushdata_inline() {
        let bytes = [0x02u8, 0xAA, 0xBB, 0x76];
        let mut s = &bytes[..];
        let i = Instruction::parse(&mut s, 0).unwrap();
        assert_eq!(i.op(), 0x02);
        assert_eq!(i.data(), &[0xAA, 0xBB]);
        assert_eq!(i.size(), 3);
        // remaining byte is OP_DUP
        let next = Instruction::parse(&mut s, 3).unwrap();
        assert_eq!(next.op(), 0x76);
    }

    #[test]
    fn test_parse_invalid_op() {
        let bytes = [0xBA, 0x00, 0x00];
        let mut s = &bytes[..];
        assert!(Instruction::parse(&mut s, 0).is_err());
    }

    #[test]
    fn test_pushdata1_round_trip() {
        let i = Instruction {
            op: 0x4c,
            data: vec![0x11, 0x22, 0x33],
            location: 0,
            size: 5,
            valid: true,
        };
        let mut out = Vec::new();
        i.serialize(&mut out);
        let mut s = &out[..];
        let parsed = Instruction::parse(&mut s, 0).unwrap();
        assert_eq!(parsed.data(), i.data());
        assert_eq!(parsed.op(), 0x4c);
    }
}
