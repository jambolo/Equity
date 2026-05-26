//! Bitcoin protocol message payloads.

use crate::address::Address;
use crate::inventory::{HASH_SIZE, InventoryId};
use crate::serialize::{
    read_array, read_u32, read_u64, read_u8, read_var_bytes, read_var_int, read_var_string,
    write_bytes, write_u32, write_u64, write_u8, write_var_bytes, write_var_int, write_var_string,
};
use anyhow::{Result, bail};
use equity::block::Block;
use equity::transaction::Transaction;
use serde_json::{Value, json};

const MAX_FILTER_ADD_SIZE: usize = 520;
const MAX_FILTER_LOAD_SIZE: usize = 36_000;
const MAX_FILTER_HASH_FUNCS: u32 = 50;

/// `version`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct VersionMessage {
    pub version: u32,
    pub services: u64,
    pub timestamp: u64,
    pub to: Address,
    pub from: Address,
    pub nonce: u64,
    pub user_agent: String,
    pub height: u32,
    pub relay: bool,
}

impl VersionMessage {
    pub const COMMAND: &'static str = "version";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.version);
        write_u64(out, self.services);
        write_u64(out, self.timestamp);
        self.to.serialize(out);
        if self.version >= 106 {
            self.from.serialize(out);
            write_u64(out, self.nonce);
            write_var_string(out, &self.user_agent);
            write_u32(out, self.height);
            if self.version >= 70001 {
                write_u8(out, self.relay as u8);
            }
        }
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let version = read_u32(stream)?;
        let services = read_u64(stream)?;
        let timestamp = read_u64(stream)?;
        let to = Address::deserialize(stream)?;
        let mut from = Address::new(0, 0, [0u8; 16], 0);
        let mut nonce = 0u64;
        let mut user_agent = String::new();
        let mut height = 0u32;
        let mut relay = true;
        if version >= 106 {
            from = Address::deserialize(stream)?;
            nonce = read_u64(stream)?;
            user_agent = read_var_string(stream)?;
            height = read_u32(stream)?;
            if version >= 70001 {
                relay = read_u8(stream)? != 0;
            }
        }
        Ok(Self {
            version,
            services,
            timestamp,
            to,
            from,
            nonce,
            user_agent,
            height,
            relay,
        })
    }

    pub fn to_json(&self) -> Value {
        let mut j = json!({
            "version": self.version,
            "services": self.services,
            "timestamp": self.timestamp,
            "to": self.to.to_json(),
        });
        if self.version >= 106 {
            j["from"] = self.from.to_json();
            j["nonce"] = json!(self.nonce);
            j["userAgent"] = json!(self.user_agent);
            j["height"] = json!(self.height);
            if self.version >= 70001 {
                j["relay"] = json!(self.relay);
            }
        }
        j
    }
}

fn serialize_inventory_list(items: &[InventoryId], out: &mut Vec<u8>) {
    write_var_int(out, items.len() as u64);
    for item in items {
        item.serialize(out);
    }
}

fn deserialize_inventory_list(stream: &mut &[u8]) -> Result<Vec<InventoryId>> {
    let n = read_var_int(stream)? as usize;
    let mut v = Vec::with_capacity(n);
    for _ in 0..n {
        v.push(InventoryId::deserialize(stream)?);
    }
    Ok(v)
}

fn serialize_hash_list(hashes: &[[u8; HASH_SIZE]], out: &mut Vec<u8>) {
    write_var_int(out, hashes.len() as u64);
    for h in hashes {
        write_bytes(out, h);
    }
}

fn deserialize_hash_list(stream: &mut &[u8]) -> Result<Vec<[u8; HASH_SIZE]>> {
    let n = read_var_int(stream)? as usize;
    let mut v = Vec::with_capacity(n);
    for _ in 0..n {
        v.push(read_array::<HASH_SIZE>(stream)?);
    }
    Ok(v)
}

/// `addr`
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct AddressMessage {
    pub addresses: Vec<Address>,
}

impl AddressMessage {
    pub const COMMAND: &'static str = "addr";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_int(out, self.addresses.len() as u64);
        for a in &self.addresses {
            a.serialize(out);
        }
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let n = read_var_int(stream)? as usize;
        let mut addresses = Vec::with_capacity(n);
        for _ in 0..n {
            addresses.push(Address::deserialize(stream)?);
        }
        Ok(Self { addresses })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "addresses": self.addresses.iter().map(Address::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `inv`
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct InventoryMessage {
    pub inventory: Vec<InventoryId>,
}

impl InventoryMessage {
    pub const COMMAND: &'static str = "inv";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.inventory, out);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            inventory: deserialize_inventory_list(stream)?,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "inventory": self.inventory.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `getdata`
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct GetDataMessage {
    pub inventory: Vec<InventoryId>,
}

impl GetDataMessage {
    pub const COMMAND: &'static str = "getdata";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.inventory, out);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            inventory: deserialize_inventory_list(stream)?,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "inventory": self.inventory.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `notfound`
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct NotFoundMessage {
    pub missing: Vec<InventoryId>,
}

impl NotFoundMessage {
    pub const COMMAND: &'static str = "notfound";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.missing, out);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            missing: deserialize_inventory_list(stream)?,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "missing": self.missing.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// Common payload for `getblocks` and `getheaders`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LocatorMessage {
    pub version: u32,
    pub hashes: Vec<[u8; HASH_SIZE]>,
    pub last: [u8; HASH_SIZE],
}

impl LocatorMessage {
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.version);
        serialize_hash_list(&self.hashes, out);
        write_bytes(out, &self.last);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let version = read_u32(stream)?;
        let hashes = deserialize_hash_list(stream)?;
        let last = read_array::<HASH_SIZE>(stream)?;
        Ok(Self {
            version,
            hashes,
            last,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "version": self.version,
            "hashes": self.hashes.iter().map(hex::encode).collect::<Vec<_>>(),
            "last": hex::encode(self.last),
        })
    }
}

/// `getblocks`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GetBlocksMessage(pub LocatorMessage);

impl GetBlocksMessage {
    pub const COMMAND: &'static str = "getblocks";
}

/// `getheaders`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GetHeadersMessage(pub LocatorMessage);

impl GetHeadersMessage {
    pub const COMMAND: &'static str = "getheaders";
}

/// `headers`
#[derive(Debug, Clone, Default)]
pub struct HeadersMessage {
    pub blocks: Vec<Block>,
}

impl HeadersMessage {
    pub const COMMAND: &'static str = "headers";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_int(out, self.blocks.len() as u64);
        for b in &self.blocks {
            out.extend_from_slice(&b.serialize());
        }
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let n = read_var_int(stream)? as usize;
        let mut blocks = Vec::with_capacity(n);
        for _ in 0..n {
            blocks.push(
                Block::deserialize(stream)
                    .map_err(|e| anyhow::anyhow!("block deserialize: {}", e))?,
            );
        }
        Ok(Self { blocks })
    }
}

/// `block`
#[derive(Debug, Clone)]
pub struct BlockMessage {
    pub block: Block,
}

impl BlockMessage {
    pub const COMMAND: &'static str = "block";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.block.serialize());
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let block = Block::deserialize(stream)
            .map_err(|e| anyhow::anyhow!("block deserialize: {}", e))?;
        Ok(Self { block })
    }
}

/// `tx`
#[derive(Debug, Clone)]
pub struct TransactionMessage {
    pub transaction: Transaction,
}

impl TransactionMessage {
    pub const COMMAND: &'static str = "tx";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.transaction.serialize());
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let transaction = Transaction::deserialize(stream)
            .map_err(|e| anyhow::anyhow!("transaction deserialize: {}", e))?;
        Ok(Self { transaction })
    }
}

/// `ping`
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PingMessage {
    pub nonce: u64,
}

impl PingMessage {
    pub const COMMAND: &'static str = "ping";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u64(out, self.nonce);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            nonce: read_u64(stream)?,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({ "nonce": self.nonce })
    }
}

/// `pong`
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PongMessage {
    pub nonce: u64,
}

impl PongMessage {
    pub const COMMAND: &'static str = "pong";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u64(out, self.nonce);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            nonce: read_u64(stream)?,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({ "nonce": self.nonce })
    }
}

/// `reject`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RejectMessage {
    pub message: String,
    pub code: u8,
    pub reason: String,
    pub data: Vec<u8>,
}

impl RejectMessage {
    pub const COMMAND: &'static str = "reject";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_string(out, &self.message);
        write_u8(out, self.code);
        write_var_string(out, &self.reason);
        write_bytes(out, &self.data);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let message = read_var_string(stream)?;
        let code = read_u8(stream)?;
        let reason = read_var_string(stream)?;
        let data = stream.to_vec();
        *stream = &stream[stream.len()..];
        Ok(Self {
            message,
            code,
            reason,
            data,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "message": self.message,
            "code": self.code,
            "reason": self.reason,
            "data": hex::encode(&self.data),
        })
    }
}

/// `filterload`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FilterLoadMessage {
    pub filter: Vec<u8>,
    pub n_hash_funcs: u32,
    pub tweak: u32,
    pub flags: u8,
}

impl FilterLoadMessage {
    pub const COMMAND: &'static str = "filterload";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_bytes(out, &self.filter);
        write_u32(out, self.n_hash_funcs);
        write_u32(out, self.tweak);
        write_u8(out, self.flags);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let filter = read_var_bytes(stream)?;
        if filter.len() > MAX_FILTER_LOAD_SIZE {
            bail!("filterload filter too large ({} bytes)", filter.len());
        }
        let n_hash_funcs = read_u32(stream)?;
        if n_hash_funcs > MAX_FILTER_HASH_FUNCS {
            bail!("filterload n_hash_funcs too large ({})", n_hash_funcs);
        }
        let tweak = read_u32(stream)?;
        let flags = read_u8(stream)?;
        Ok(Self {
            filter,
            n_hash_funcs,
            tweak,
            flags,
        })
    }

    pub fn to_json(&self) -> Value {
        json!({
            "filter": hex::encode(&self.filter),
            "num_hash_funcs": self.n_hash_funcs,
            "tweak": self.tweak,
            "flags": self.flags,
        })
    }
}

/// `filteradd`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FilterAddMessage {
    pub data: Vec<u8>,
}

impl FilterAddMessage {
    pub const COMMAND: &'static str = "filteradd";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_bytes(out, &self.data);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let data = read_var_bytes(stream)?;
        if data.len() > MAX_FILTER_ADD_SIZE {
            bail!("filteradd data too large ({} bytes)", data.len());
        }
        Ok(Self { data })
    }

    pub fn to_json(&self) -> Value {
        json!({ "data": hex::encode(&self.data) })
    }
}

/// `merkleblock`
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MerkleBlockMessage {
    pub header: equity::block::BlockHeader,
    pub total_transactions: u32,
    pub hashes: Vec<[u8; HASH_SIZE]>,
    pub flags: Vec<u8>,
}

impl MerkleBlockMessage {
    pub const COMMAND: &'static str = "merkleblock";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        self.header.serialize(out);
        write_u32(out, self.total_transactions);
        serialize_hash_list(&self.hashes, out);
        write_var_bytes(out, &self.flags);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let header = equity::block::BlockHeader::deserialize(stream)
            .map_err(|e| anyhow::anyhow!("merkleblock header: {}", e))?;
        let total_transactions = read_u32(stream)?;
        let hashes = deserialize_hash_list(stream)?;
        let flags = read_var_bytes(stream)?;
        Ok(Self {
            header,
            total_transactions,
            hashes,
            flags,
        })
    }
}

/// `alert`
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct AlertMessage {
    pub message: Vec<u8>,
}

impl AlertMessage {
    pub const COMMAND: &'static str = "alert";

    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_bytes(out, &self.message);
    }

    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let message = stream.to_vec();
        *stream = &stream[stream.len()..];
        Ok(Self { message })
    }

    pub fn to_json(&self) -> Value {
        json!({ "message": hex::encode(&self.message) })
    }
}

/// Discriminated enum over every supported Bitcoin message type.
#[derive(Debug, Clone)]
pub enum Message {
    Version(VersionMessage),
    Verack,
    Addr(AddressMessage),
    Inv(InventoryMessage),
    GetData(GetDataMessage),
    NotFound(NotFoundMessage),
    GetBlocks(GetBlocksMessage),
    GetHeaders(GetHeadersMessage),
    Headers(HeadersMessage),
    Block(BlockMessage),
    Tx(TransactionMessage),
    Ping(PingMessage),
    Pong(PongMessage),
    GetAddr,
    Reject(RejectMessage),
    FilterLoad(FilterLoadMessage),
    FilterAdd(FilterAddMessage),
    FilterClear,
    MerkleBlock(MerkleBlockMessage),
    Alert(AlertMessage),
    SendHeaders,
    Mempool,
}

impl Message {
    pub fn command(&self) -> &'static str {
        match self {
            Self::Version(_) => VersionMessage::COMMAND,
            Self::Verack => "verack",
            Self::Addr(_) => AddressMessage::COMMAND,
            Self::Inv(_) => InventoryMessage::COMMAND,
            Self::GetData(_) => GetDataMessage::COMMAND,
            Self::NotFound(_) => NotFoundMessage::COMMAND,
            Self::GetBlocks(_) => GetBlocksMessage::COMMAND,
            Self::GetHeaders(_) => GetHeadersMessage::COMMAND,
            Self::Headers(_) => HeadersMessage::COMMAND,
            Self::Block(_) => BlockMessage::COMMAND,
            Self::Tx(_) => TransactionMessage::COMMAND,
            Self::Ping(_) => PingMessage::COMMAND,
            Self::Pong(_) => PongMessage::COMMAND,
            Self::GetAddr => "getaddr",
            Self::Reject(_) => RejectMessage::COMMAND,
            Self::FilterLoad(_) => FilterLoadMessage::COMMAND,
            Self::FilterAdd(_) => FilterAddMessage::COMMAND,
            Self::FilterClear => "filterclear",
            Self::MerkleBlock(_) => MerkleBlockMessage::COMMAND,
            Self::Alert(_) => AlertMessage::COMMAND,
            Self::SendHeaders => "sendheaders",
            Self::Mempool => "mempool",
        }
    }

    pub fn serialize_payload(&self, out: &mut Vec<u8>) {
        match self {
            Self::Version(m) => m.serialize(out),
            Self::Addr(m) => m.serialize(out),
            Self::Inv(m) => m.serialize(out),
            Self::GetData(m) => m.serialize(out),
            Self::NotFound(m) => m.serialize(out),
            Self::GetBlocks(m) => m.0.serialize(out),
            Self::GetHeaders(m) => m.0.serialize(out),
            Self::Headers(m) => m.serialize(out),
            Self::Block(m) => m.serialize(out),
            Self::Tx(m) => m.serialize(out),
            Self::Ping(m) => m.serialize(out),
            Self::Pong(m) => m.serialize(out),
            Self::Reject(m) => m.serialize(out),
            Self::FilterLoad(m) => m.serialize(out),
            Self::FilterAdd(m) => m.serialize(out),
            Self::MerkleBlock(m) => m.serialize(out),
            Self::Alert(m) => m.serialize(out),
            Self::Verack
            | Self::GetAddr
            | Self::FilterClear
            | Self::SendHeaders
            | Self::Mempool => {}
        }
    }

    pub fn payload_bytes(&self) -> Vec<u8> {
        let mut out = Vec::new();
        self.serialize_payload(&mut out);
        out
    }

    pub fn deserialize_payload(command: &str, payload: &[u8]) -> Result<Self> {
        let mut s = payload;
        let m = match command {
            "version" => Self::Version(VersionMessage::deserialize(&mut s)?),
            "verack" => Self::Verack,
            "addr" => Self::Addr(AddressMessage::deserialize(&mut s)?),
            "inv" => Self::Inv(InventoryMessage::deserialize(&mut s)?),
            "getdata" => Self::GetData(GetDataMessage::deserialize(&mut s)?),
            "notfound" => Self::NotFound(NotFoundMessage::deserialize(&mut s)?),
            "getblocks" => Self::GetBlocks(GetBlocksMessage(LocatorMessage::deserialize(&mut s)?)),
            "getheaders" => {
                Self::GetHeaders(GetHeadersMessage(LocatorMessage::deserialize(&mut s)?))
            }
            "headers" => Self::Headers(HeadersMessage::deserialize(&mut s)?),
            "block" => Self::Block(BlockMessage::deserialize(&mut s)?),
            "tx" => Self::Tx(TransactionMessage::deserialize(&mut s)?),
            "ping" => Self::Ping(PingMessage::deserialize(&mut s)?),
            "pong" => Self::Pong(PongMessage::deserialize(&mut s)?),
            "getaddr" => Self::GetAddr,
            "reject" => Self::Reject(RejectMessage::deserialize(&mut s)?),
            "filterload" => Self::FilterLoad(FilterLoadMessage::deserialize(&mut s)?),
            "filteradd" => Self::FilterAdd(FilterAddMessage::deserialize(&mut s)?),
            "filterclear" => Self::FilterClear,
            "merkleblock" => Self::MerkleBlock(MerkleBlockMessage::deserialize(&mut s)?),
            "alert" => Self::Alert(AlertMessage::deserialize(&mut s)?),
            "sendheaders" => Self::SendHeaders,
            "mempool" => Self::Mempool,
            _ => bail!("unknown message type '{}'", command),
        };
        Ok(m)
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::address::Address;
    use crate::inventory::{InventoryId, InventoryType};

    #[test]
    fn ping_round_trip() {
        let m = PingMessage { nonce: 0xCAFE_BABE_DEAD_BEEF };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        assert_eq!(bytes.len(), 8);
        let parsed = PingMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn version_round_trip() {
        let m = VersionMessage {
            version: 70015,
            services: 1,
            timestamp: 1_700_000_000,
            to: Address::from_ipv4(0, 1, [127, 0, 0, 1], 8333),
            from: Address::from_ipv4(0, 1, [10, 0, 0, 1], 8333),
            nonce: 0x1234_5678_9abc_def0,
            user_agent: "/equity:0.1/".to_string(),
            height: 800_000,
            relay: true,
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = VersionMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn inv_round_trip() {
        let m = InventoryMessage {
            inventory: vec![
                InventoryId::new(InventoryType::Transaction, [0x11u8; 32]),
                InventoryId::new(InventoryType::Block, [0x22u8; 32]),
            ],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = InventoryMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn reject_round_trip() {
        let m = RejectMessage {
            message: "tx".to_string(),
            code: 0x10,
            reason: "invalid".to_string(),
            data: vec![1, 2, 3, 4],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = RejectMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn filterload_round_trip() {
        let m = FilterLoadMessage {
            filter: vec![0xAA, 0xBB, 0xCC],
            n_hash_funcs: 11,
            tweak: 0xDEAD_BEEF,
            flags: 1,
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = FilterLoadMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn filterload_rejects_too_many_hash_funcs() {
        let mut bytes = Vec::new();
        write_var_bytes(&mut bytes, &[0u8; 10]);
        write_u32(&mut bytes, MAX_FILTER_HASH_FUNCS + 1);
        write_u32(&mut bytes, 0);
        write_u8(&mut bytes, 0);
        assert!(FilterLoadMessage::deserialize(&mut &bytes[..]).is_err());
    }

    #[test]
    fn locator_round_trip() {
        let m = LocatorMessage {
            version: 70015,
            hashes: vec![[1u8; 32], [2u8; 32]],
            last: [0xFFu8; 32],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = LocatorMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn message_enum_dispatch() {
        let v = Message::Verack;
        assert_eq!(v.command(), "verack");
        assert!(v.payload_bytes().is_empty());
        let dec = Message::deserialize_payload("verack", &[]).unwrap();
        assert!(matches!(dec, Message::Verack));
    }

    #[test]
    fn unknown_command_errors() {
        assert!(Message::deserialize_payload("nope", &[]).is_err());
    }

}
