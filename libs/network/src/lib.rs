//! Bitcoin network layer — pure Rust implementation of message types,
//! serialization, and the wire envelope used by the P2P protocol.

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
