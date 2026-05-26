//! Bitcoin network layer — message payloads, serialization helpers, and the
//! wire envelope used by the P2P protocol.
//!
//! The crate is organized as:
//!
//! - [`serialize`] — primitive readers/writers for little-endian integers,
//!   CompactSize varints, var-strings, and var-bytes.
//! - [`address`] — 30-byte on-wire network address with IPv4-in-IPv6 mapping.
//! - [`inventory`] — `inv` / `getdata` / `notfound` entry type.
//! - [`header`] — 24-byte P2P message header (magic, command, length, checksum).
//! - [`messages`] — strongly-typed enums and structs for every supported
//!   Bitcoin P2P message (`version`, `verack`, `inv`, `tx`, `block`, …).
//! - [`wire`] — [`WireMessage`] framing: header + payload + checksum.
//!
//! # Examples
//!
//! Round-trip a `ping` over the wire:
//!
//! ```
//! use network::{Message, WireMessage};
//! use network::messages::PingMessage;
//! use network::header::MAGIC_MAIN;
//!
//! let msg = Message::Ping(PingMessage { nonce: 42 });
//! let wire = WireMessage::encode(&msg, MAGIC_MAIN).unwrap();
//! let bytes = wire.to_bytes();
//!
//! let parsed = WireMessage::from_bytes(&bytes).unwrap();
//! match parsed.decode().unwrap() {
//!     Message::Ping(p) => assert_eq!(p.nonce, 42),
//!     _ => panic!("wrong message type"),
//! }
//! ```

pub mod address;
pub mod configuration;
pub mod header;
pub mod inventory;
pub mod messages;
pub mod serialize;
pub mod wire;

pub use address::Address;
pub use configuration::{Configuration, get_network, get_port};
pub use header::{COMMAND_SIZE, HEADER_SIZE, Header, MAGIC_MAIN, MAGIC_TEST, MAGIC_TEST3};
pub use inventory::{HASH_SIZE, InventoryId, InventoryType};
pub use messages::{
    AddressMessage, AlertMessage, BlockMessage, FilterAddMessage, FilterLoadMessage,
    GetBlocksMessage, GetDataMessage, GetHeadersMessage, HeadersMessage, InventoryMessage,
    LocatorMessage, MerkleBlockMessage, Message, NotFoundMessage, PingMessage, PongMessage,
    RejectMessage, TransactionMessage, VersionMessage,
};
pub use wire::WireMessage;
