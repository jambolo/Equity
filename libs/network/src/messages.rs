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

/// `version` — initial handshake message.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct VersionMessage {
    /// Protocol version.
    pub version: u32,
    /// Bitmask of services offered by the sender.
    pub services: u64,
    /// Unix timestamp.
    pub timestamp: u64,
    /// Recipient's address as seen by the sender.
    pub to: Address,
    /// Sender's own address (only meaningful for `version >= 106`).
    pub from: Address,
    /// Random nonce used to detect self-connections.
    pub nonce: u64,
    /// Sender's user agent string.
    pub user_agent: String,
    /// Best block height known to the sender.
    pub height: u32,
    /// `false` requests no transaction relay (`version >= 70001`).
    pub relay: bool,
}

impl VersionMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "version";

    /// Append the on-wire encoding of this message to `out`.
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

    /// Parse from `stream`, advancing the cursor.
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

    /// JSON representation of this message.
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

/// `addr` — list of known peers.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct AddressMessage {
    /// Known peer addresses.
    pub addresses: Vec<Address>,
}

impl AddressMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "addr";

    /// Append the on-wire encoding of this message to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_int(out, self.addresses.len() as u64);
        for a in &self.addresses {
            a.serialize(out);
        }
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let n = read_var_int(stream)? as usize;
        let mut addresses = Vec::with_capacity(n);
        for _ in 0..n {
            addresses.push(Address::deserialize(stream)?);
        }
        Ok(Self { addresses })
    }

    /// JSON representation of this message.
    pub fn to_json(&self) -> Value {
        json!({
            "addresses": self.addresses.iter().map(Address::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `inv` — advertise known inventory items.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct InventoryMessage {
    /// Advertised inventory items.
    pub inventory: Vec<InventoryId>,
}

impl InventoryMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "inv";

    /// Append the on-wire encoding of this message to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.inventory, out);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            inventory: deserialize_inventory_list(stream)?,
        })
    }

    /// JSON representation of this message.
    pub fn to_json(&self) -> Value {
        json!({
            "inventory": self.inventory.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `getdata` — request the full contents of inventory items.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct GetDataMessage {
    /// Items being requested.
    pub inventory: Vec<InventoryId>,
}

impl GetDataMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "getdata";

    /// Append the on-wire encoding of this message to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.inventory, out);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            inventory: deserialize_inventory_list(stream)?,
        })
    }

    /// JSON representation of this message.
    pub fn to_json(&self) -> Value {
        json!({
            "inventory": self.inventory.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// `notfound` — peer could not supply requested inventory.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct NotFoundMessage {
    /// Items the peer could not provide.
    pub missing: Vec<InventoryId>,
}

impl NotFoundMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "notfound";

    /// Append the on-wire encoding of this message to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        serialize_inventory_list(&self.missing, out);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            missing: deserialize_inventory_list(stream)?,
        })
    }

    /// JSON representation of this message.
    pub fn to_json(&self) -> Value {
        json!({
            "missing": self.missing.iter().map(InventoryId::to_json).collect::<Vec<_>>(),
        })
    }
}

/// Common payload for `getblocks` and `getheaders`.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct LocatorMessage {
    /// Protocol version of the sender.
    pub version: u32,
    /// Block locator hashes from tip back toward genesis.
    pub hashes: Vec<[u8; HASH_SIZE]>,
    /// Hash of the last desired block (zero means "as many as possible").
    pub last: [u8; HASH_SIZE],
}

impl LocatorMessage {
    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u32(out, self.version);
        serialize_hash_list(&self.hashes, out);
        write_bytes(out, &self.last);
    }

    /// Parse from `stream`, advancing the cursor.
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

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({
            "version": self.version,
            "hashes": self.hashes.iter().map(hex::encode).collect::<Vec<_>>(),
            "last": hex::encode(self.last),
        })
    }
}

/// `getblocks` — request blocks announced via a locator.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GetBlocksMessage(
    /// Locator payload.
    pub LocatorMessage,
);

impl GetBlocksMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "getblocks";
}

/// `getheaders` — request block headers via a locator.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct GetHeadersMessage(
    /// Locator payload.
    pub LocatorMessage,
);

impl GetHeadersMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "getheaders";
}

/// `headers` — response to `getheaders` carrying block headers.
#[derive(Debug, Clone, Default)]
pub struct HeadersMessage {
    /// Header-only blocks (transactions list is empty).
    pub blocks: Vec<Block>,
}

impl HeadersMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "headers";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_int(out, self.blocks.len() as u64);
        for b in &self.blocks {
            out.extend_from_slice(&b.serialize());
        }
    }

    /// Parse from `stream`, advancing the cursor.
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

/// `block` — a single full block.
#[derive(Debug, Clone)]
pub struct BlockMessage {
    /// Wrapped block.
    pub block: Block,
}

impl BlockMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "block";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.block.serialize());
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let block = Block::deserialize(stream)
            .map_err(|e| anyhow::anyhow!("block deserialize: {}", e))?;
        Ok(Self { block })
    }
}

/// `tx` — a single transaction.
#[derive(Debug, Clone)]
pub struct TransactionMessage {
    /// Wrapped transaction.
    pub transaction: Transaction,
}

impl TransactionMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "tx";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        out.extend_from_slice(&self.transaction.serialize());
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let transaction = Transaction::deserialize(stream)
            .map_err(|e| anyhow::anyhow!("transaction deserialize: {}", e))?;
        Ok(Self { transaction })
    }
}

/// `ping` — keep-alive probe.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PingMessage {
    /// Random nonce; expected back in the matching `pong`.
    pub nonce: u64,
}

impl PingMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "ping";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u64(out, self.nonce);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            nonce: read_u64(stream)?,
        })
    }

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({ "nonce": self.nonce })
    }
}

/// `pong` — response to a `ping`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct PongMessage {
    /// Nonce echoed from the matching `ping`.
    pub nonce: u64,
}

impl PongMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "pong";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_u64(out, self.nonce);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        Ok(Self {
            nonce: read_u64(stream)?,
        })
    }

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({ "nonce": self.nonce })
    }
}

/// `reject` — peer rejection of a prior message.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct RejectMessage {
    /// Command name being rejected.
    pub message: String,
    /// Reject reason code (see Bitcoin Core `RejectCode`).
    pub code: u8,
    /// Human-readable explanation.
    pub reason: String,
    /// Optional extra data (e.g. txid for `tx` rejects).
    pub data: Vec<u8>,
}

impl RejectMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "reject";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_string(out, &self.message);
        write_u8(out, self.code);
        write_var_string(out, &self.reason);
        write_bytes(out, &self.data);
    }

    /// Parse from `stream`, advancing the cursor.
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

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({
            "message": self.message,
            "code": self.code,
            "reason": self.reason,
            "data": hex::encode(&self.data),
        })
    }
}

/// `filterload` — install a Bloom filter for SPV-style relay (BIP-37).
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FilterLoadMessage {
    /// Bloom filter bytes.
    pub filter: Vec<u8>,
    /// Number of hash functions.
    pub n_hash_funcs: u32,
    /// Random tweak mixed into each hash function.
    pub tweak: u32,
    /// `BLOOM_UPDATE_*` flag byte.
    pub flags: u8,
}

impl FilterLoadMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "filterload";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_bytes(out, &self.filter);
        write_u32(out, self.n_hash_funcs);
        write_u32(out, self.tweak);
        write_u8(out, self.flags);
    }

    /// Parse from `stream`, advancing the cursor.
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

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({
            "filter": hex::encode(&self.filter),
            "num_hash_funcs": self.n_hash_funcs,
            "tweak": self.tweak,
            "flags": self.flags,
        })
    }
}

/// `filteradd` — extend the active Bloom filter with one more element.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FilterAddMessage {
    /// Element to add to the filter.
    pub data: Vec<u8>,
}

impl FilterAddMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "filteradd";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_var_bytes(out, &self.data);
    }

    /// Parse from `stream`, advancing the cursor.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let data = read_var_bytes(stream)?;
        if data.len() > MAX_FILTER_ADD_SIZE {
            bail!("filteradd data too large ({} bytes)", data.len());
        }
        Ok(Self { data })
    }

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({ "data": hex::encode(&self.data) })
    }
}

/// `merkleblock` — partial Merkle proof of transactions matching a Bloom filter.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct MerkleBlockMessage {
    /// Block header.
    pub header: equity::block::BlockHeader,
    /// Total transaction count in the full block.
    pub total_transactions: u32,
    /// Hashes used to reconstruct the partial Merkle tree.
    pub hashes: Vec<[u8; HASH_SIZE]>,
    /// Bit flags describing the partial-tree shape.
    pub flags: Vec<u8>,
}

impl MerkleBlockMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "merkleblock";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        self.header.serialize(out);
        write_u32(out, self.total_transactions);
        serialize_hash_list(&self.hashes, out);
        write_var_bytes(out, &self.flags);
    }

    /// Parse from `stream`, advancing the cursor.
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

/// `alert` — deprecated network-wide alert payload.
#[derive(Debug, Clone, PartialEq, Eq, Default)]
pub struct AlertMessage {
    /// Raw signed alert blob.
    pub message: Vec<u8>,
}

impl AlertMessage {
    /// Wire command name.
    pub const COMMAND: &'static str = "alert";

    /// Append the on-wire encoding to `out`.
    pub fn serialize(&self, out: &mut Vec<u8>) {
        write_bytes(out, &self.message);
    }

    /// Parse from `stream`, consuming all remaining bytes.
    pub fn deserialize(stream: &mut &[u8]) -> Result<Self> {
        let message = stream.to_vec();
        *stream = &stream[stream.len()..];
        Ok(Self { message })
    }

    /// JSON representation.
    pub fn to_json(&self) -> Value {
        json!({ "message": hex::encode(&self.message) })
    }
}

/// Discriminated enum over every supported Bitcoin message type.
#[derive(Debug, Clone)]
pub enum Message {
    /// `version`
    Version(VersionMessage),
    /// `verack` — version handshake acknowledgement.
    Verack,
    /// `addr`
    Addr(AddressMessage),
    /// `inv`
    Inv(InventoryMessage),
    /// `getdata`
    GetData(GetDataMessage),
    /// `notfound`
    NotFound(NotFoundMessage),
    /// `getblocks`
    GetBlocks(GetBlocksMessage),
    /// `getheaders`
    GetHeaders(GetHeadersMessage),
    /// `headers`
    Headers(HeadersMessage),
    /// `block`
    Block(BlockMessage),
    /// `tx`
    Tx(TransactionMessage),
    /// `ping`
    Ping(PingMessage),
    /// `pong`
    Pong(PongMessage),
    /// `getaddr` — request peer addresses.
    GetAddr,
    /// `reject`
    Reject(RejectMessage),
    /// `filterload`
    FilterLoad(FilterLoadMessage),
    /// `filteradd`
    FilterAdd(FilterAddMessage),
    /// `filterclear` — drop the active Bloom filter.
    FilterClear,
    /// `merkleblock`
    MerkleBlock(MerkleBlockMessage),
    /// `alert`
    Alert(AlertMessage),
    /// `sendheaders` — request that future block announcements use `headers`.
    SendHeaders,
    /// `mempool` — request the peer's mempool TXIDs.
    Mempool,
}

impl Message {
    /// Wire command name for this message variant.
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

    /// Append this message's payload (without the wire header) to `out`.
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

    /// Payload bytes for this message as a fresh `Vec`.
    pub fn payload_bytes(&self) -> Vec<u8> {
        let mut out = Vec::new();
        self.serialize_payload(&mut out);
        out
    }

    /// Decode `payload` into the message matching `command`. Errors on unknown commands.
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

    #[test]
    fn addr_message_round_trip() {
        let m = AddressMessage {
            addresses: vec![
                Address::from_ipv4(0, 1, [127, 0, 0, 1], 8333),
                Address::from_ipv4(1, 2, [192, 168, 1, 1], 18333),
            ],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = AddressMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn getdata_message_round_trip() {
        let m = GetDataMessage {
            inventory: vec![InventoryId::transaction([0x33u8; 32])],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = GetDataMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn notfound_message_round_trip() {
        let m = NotFoundMessage {
            missing: vec![
                InventoryId::filtered_block([0x44u8; 32]),
                InventoryId::block([0x55u8; 32]),
            ],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = NotFoundMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn pong_round_trip() {
        let m = PongMessage { nonce: 0xDEAD_BEEF_F00D_BABE };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        assert_eq!(bytes.len(), 8);
        let parsed = PongMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn filteradd_round_trip() {
        let m = FilterAddMessage {
            data: vec![0xAB, 0xCD, 0xEF],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = FilterAddMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn filteradd_rejects_oversize() {
        let mut bytes = Vec::new();
        let big = vec![0u8; MAX_FILTER_ADD_SIZE + 1];
        write_var_bytes(&mut bytes, &big);
        assert!(FilterAddMessage::deserialize(&mut &bytes[..]).is_err());
    }

    #[test]
    fn filterload_rejects_oversize_filter() {
        let mut bytes = Vec::new();
        let big = vec![0u8; MAX_FILTER_LOAD_SIZE + 1];
        write_var_bytes(&mut bytes, &big);
        write_u32(&mut bytes, 1);
        write_u32(&mut bytes, 0);
        write_u8(&mut bytes, 0);
        assert!(FilterLoadMessage::deserialize(&mut &bytes[..]).is_err());
    }

    #[test]
    fn alert_round_trip() {
        let m = AlertMessage {
            message: vec![1, 2, 3, 4, 5],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = AlertMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn merkleblock_round_trip() {
        let header = equity::block::BlockHeader {
            version: 1,
            previous_block: [0xAAu8; 32],
            merkle_root: [0xBBu8; 32],
            timestamp: 1_700_000_000,
            target: 0x1d00ffff,
            nonce: 7,
        };
        let m = MerkleBlockMessage {
            header,
            total_transactions: 3,
            hashes: vec![[0x11u8; 32], [0x22u8; 32]],
            flags: vec![0b0000_0001],
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = MerkleBlockMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(m, parsed);
    }

    #[test]
    fn version_pre_106_skips_optional_fields() {
        let m = VersionMessage {
            version: 105,
            services: 0,
            timestamp: 0,
            to: Address::from_ipv4(0, 0, [0, 0, 0, 0], 0),
            from: Address::from_ipv4(0, 0, [0, 0, 0, 0], 0),
            nonce: 0,
            user_agent: String::new(),
            height: 0,
            relay: true,
        };
        let mut bytes = Vec::new();
        m.serialize(&mut bytes);
        let parsed = VersionMessage::deserialize(&mut &bytes[..]).unwrap();
        assert_eq!(parsed.version, 105);
        assert_eq!(parsed.user_agent, "");
    }

    #[test]
    fn message_enum_command_strings() {
        // Sanity: each variant returns a non-empty unique command name.
        let variants: Vec<Message> = vec![
            Message::Verack,
            Message::GetAddr,
            Message::FilterClear,
            Message::SendHeaders,
            Message::Mempool,
        ];
        let mut names = std::collections::HashSet::new();
        for v in &variants {
            let c = v.command();
            assert!(!c.is_empty());
            assert!(names.insert(c));
        }
    }

    proptest::proptest! {
        #[test]
        fn prop_ping_round_trip(nonce: u64) {
            let m = PingMessage { nonce };
            let mut bytes = Vec::new();
            m.serialize(&mut bytes);
            let parsed = PingMessage::deserialize(&mut &bytes[..]).unwrap();
            proptest::prop_assert_eq!(m, parsed);
        }

        #[test]
        fn prop_inv_round_trip(items in proptest::collection::vec(any::<[u8; 32]>(), 0..16)) {
            let m = InventoryMessage {
                inventory: items.iter().map(|h| InventoryId::transaction(*h)).collect(),
            };
            let mut bytes = Vec::new();
            m.serialize(&mut bytes);
            let parsed = InventoryMessage::deserialize(&mut &bytes[..]).unwrap();
            proptest::prop_assert_eq!(m, parsed);
        }

        #[test]
        fn prop_reject_round_trip(
            message in "[a-z]{1,12}",
            code: u8,
            reason in "[ -~]{0,80}",
            data in proptest::collection::vec(any::<u8>(), 0..32),
        ) {
            let m = RejectMessage { message, code, reason, data };
            let mut bytes = Vec::new();
            m.serialize(&mut bytes);
            let parsed = RejectMessage::deserialize(&mut &bytes[..]).unwrap();
            proptest::prop_assert_eq!(m, parsed);
        }
    }

    use proptest::prelude::any;
}
