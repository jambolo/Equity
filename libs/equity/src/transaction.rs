//! Bitcoin transaction — moves coins from prior outputs to new ones.
//!
//! Wire format (version 1):
//!   u32 version_le
//!   varint input_count
//!   [Input]
//!   varint output_count
//!   [Output]
//!   u32 locktime_le

use crate::txid::{Txid, TXID_SIZE};
use crate::{EquityError, Result};
use p2p::{deserialize_var_int, serialize_var_int};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Input {
    pub txid: Txid,
    pub output_index: u32,
    pub script: Vec<u8>,
    pub sequence: u32,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Output {
    pub value: u64,
    pub script: Vec<u8>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct Transaction {
    version: u32,
    inputs: Vec<Input>,
    outputs: Vec<Output>,
    lock_time: u32,
    valid: bool,
}

impl Input {
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let txid = Txid::deserialize(stream)?;
        let output_index = read_u32_le(stream)?;
        let script = read_varbytes(stream)?;
        let sequence = read_u32_le(stream)?;
        Ok(Self {
            txid,
            output_index,
            script,
            sequence,
        })
    }

    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.txid.serialize());
        out.extend_from_slice(&self.output_index.to_le_bytes());
        out.extend_from_slice(&serialize_var_int(self.script.len() as u64));
        out.extend_from_slice(&self.script);
        out.extend_from_slice(&self.sequence.to_le_bytes());
    }
}

impl Output {
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let value = read_u64_le(stream)?;
        let script = read_varbytes(stream)?;
        Ok(Self { value, script })
    }

    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.value.to_le_bytes());
        out.extend_from_slice(&serialize_var_int(self.script.len() as u64));
        out.extend_from_slice(&self.script);
    }
}

impl Transaction {
    pub fn new(version: u32, inputs: Vec<Input>, outputs: Vec<Output>, lock_time: u32) -> Self {
        Self {
            version,
            inputs,
            outputs,
            lock_time,
            valid: true,
        }
    }

    pub fn from_data(data: &[u8]) -> Result<Self> {
        let mut stream = data;
        Self::deserialize(&mut stream)
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let version = read_u32_le(stream)?;
        if version != 1 {
            return Err(EquityError(format!("Unsupported transaction version: {version}")));
        }
        let input_count = read_var_int(stream)? as usize;
        let mut inputs = Vec::with_capacity(input_count);
        for _ in 0..input_count {
            inputs.push(Input::deserialize(stream)?);
        }
        let output_count = read_var_int(stream)? as usize;
        let mut outputs = Vec::with_capacity(output_count);
        for _ in 0..output_count {
            outputs.push(Output::deserialize(stream)?);
        }
        let lock_time = read_u32_le(stream)?;
        Ok(Self {
            version,
            inputs,
            outputs,
            lock_time,
            valid: true,
        })
    }

    pub fn serialize(&self) -> Vec<u8> {
        let mut out = Vec::new();
        out.extend_from_slice(&self.version.to_le_bytes());
        out.extend_from_slice(&serialize_var_int(self.inputs.len() as u64));
        for input in &self.inputs {
            input.serialize(&mut out);
        }
        out.extend_from_slice(&serialize_var_int(self.outputs.len() as u64));
        for output in &self.outputs {
            output.serialize(&mut out);
        }
        out.extend_from_slice(&self.lock_time.to_le_bytes());
        out
    }

    pub fn version(&self) -> u32 {
        self.version
    }

    pub fn lock_time(&self) -> u32 {
        self.lock_time
    }

    pub fn inputs(&self) -> &[Input] {
        &self.inputs
    }

    pub fn outputs(&self) -> &[Output] {
        &self.outputs
    }

    pub fn input_count(&self) -> usize {
        self.inputs.len()
    }

    pub fn output_count(&self) -> usize {
        self.outputs.len()
    }

    pub fn get_input(&self, i: usize) -> Option<&Input> {
        self.inputs.get(i)
    }

    pub fn get_output(&self, i: usize) -> Option<&Output> {
        self.outputs.get(i)
    }

    pub fn is_valid(&self) -> bool {
        self.valid
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

fn read_u64_le(stream: &mut &[u8]) -> Result<u64> {
    if stream.len() < 8 {
        return Err(EquityError("Truncated u64".to_string()));
    }
    let (head, rest) = stream.split_at(8);
    let mut buf = [0u8; 8];
    buf.copy_from_slice(head);
    let v = u64::from_le_bytes(buf);
    *stream = rest;
    Ok(v)
}

fn read_var_int(stream: &mut &[u8]) -> Result<u64> {
    let r = deserialize_var_int(stream);
    if r.bytes_read == 0 {
        return Err(EquityError("Truncated varint".to_string()));
    }
    *stream = &stream[r.bytes_read..];
    Ok(r.value)
}

fn read_varbytes(stream: &mut &[u8]) -> Result<Vec<u8>> {
    let len = read_var_int(stream)? as usize;
    if stream.len() < len {
        return Err(EquityError("Truncated varbytes".to_string()));
    }
    let (head, rest) = stream.split_at(len);
    let v = head.to_vec();
    *stream = rest;
    Ok(v)
}

// Silence unused warning when TXID_SIZE is referenced only via Txid::deserialize.
const _: () = {
    let _ = TXID_SIZE;
};

#[cfg(test)]
mod tests {
    use super::*;

    fn dummy_txid() -> Txid {
        Txid::from_data(&[0x77u8; 32]).unwrap()
    }

    #[test]
    fn test_serialize_round_trip_minimal() {
        let tx = Transaction::new(
            1,
            vec![Input {
                txid: dummy_txid(),
                output_index: 0,
                script: vec![0x51],
                sequence: 0xffffffff,
            }],
            vec![Output {
                value: 5000_0000,
                script: vec![0x76, 0xa9, 0x14],
            }],
            0,
        );
        let bytes = tx.serialize();
        let parsed = Transaction::from_data(&bytes).unwrap();
        assert_eq!(parsed, tx);
    }

    #[test]
    fn test_unsupported_version_rejected() {
        let mut bytes = vec![];
        bytes.extend_from_slice(&2u32.to_le_bytes()); // version=2
        bytes.push(0); // 0 inputs
        bytes.push(0); // 0 outputs
        bytes.extend_from_slice(&0u32.to_le_bytes()); // locktime
        assert!(Transaction::from_data(&bytes).is_err());
    }

    #[test]
    fn test_truncated_data_rejected() {
        assert!(Transaction::from_data(&[1u8, 0, 0]).is_err());
    }

    #[test]
    fn test_genesis_coinbase_round_trip() {
        // Bitcoin genesis block coinbase transaction (hand-built).
        // version=1, 1 input (all-zero txid, idx=0xffffffff, scriptSig=4d04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73),
        // sequence=0xffffffff, 1 output (50 BTC, scriptPubKey=4104...ac), locktime=0.
        let hex = "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4d04ffff001d0104455468652054696d65732030332f4a616e2f32303039204368616e63656c6c6f72206f6e206272696e6b206f66207365636f6e64206261696c6f757420666f722062616e6b73ffffffff0100f2052a01000000434104678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5fac00000000";
        let bytes = hex::decode(hex).unwrap();
        let tx = Transaction::from_data(&bytes).unwrap();
        assert_eq!(tx.version(), 1);
        assert_eq!(tx.input_count(), 1);
        assert_eq!(tx.output_count(), 1);
        assert_eq!(tx.outputs()[0].value, 50_0000_0000);
        // Round-trip exact bytes.
        assert_eq!(tx.serialize(), bytes);
    }
}
