# Equity — Technical Design Document

Version: 0.1 (draft)
Status: living document, updated as the project advances.
Audience: human contributors and future automated agents (Claude, etc.).
Scope: a from-scratch Bitcoin **node** in Rust. **No integrated wallet.**

This document describes the intended end-state architecture and explicitly labels which parts are implemented today, which are designed but not built, and which are still open questions. Where the project has not yet made a choice, the document records what the leading reference implementations (Bitcoin Core, btcd, libbitcoin, Floresta) do so future agents have a starting point.

---

## 1. Goals and non-goals

### 1.1 Goals

1. **Validate the canonical Bitcoin mainnet chain** from genesis. Pass consensus rules sufficient to stay on the longest valid chain.
2. **Speak the Bitcoin P2P protocol** over TCP. Connect to peers, exchange `version`/`verack`, perform headers-first sync, relay transactions and blocks.
3. **Serve as a learning exercise.** Idiomatic Rust, minimal external surface, easy to read. Code quality outranks raw performance on first pass.
4. **Be embeddable as a library.** The crates expose normal Rust APIs; the node binary is one consumer.
5. **Run on Windows, Linux, and macOS.** Single codebase. OS-specific paths only where necessary (data dir, file I/O backend).

### 1.2 Non-goals

1. **No wallet.** Equity does not manage keys, derive addresses for users, build transactions to send funds, or persist any spend authority. The `equity::wallet` module that exists today is a small in-memory bag of key triples used in tests and examples; it will be removed or moved out of the protocol crates before 1.0.
2. **No mining.** PoW verification is in scope; PoW production is not.
3. **No SegWit / Taproot in v0.1.** Pre-SegWit (version 1, non-witness) is the initial target. SegWit / BIP-141 / Taproot are explicit follow-ups.
4. **No GUI.** CLI binaries and (eventually) an RPC server.
5. **No alternative consensus.** Mainnet rules only. Testnet/regtest selectable for development but no signet, no custom forks.

### 1.3 Anti-goals (things we will not adopt)

- A persistent UTXO format compatible with Bitcoin Core. Equity uses its own schema.
- A pluggable consensus engine. The chain validator is monolithic.
- Re-implementation of `secp256k1`. Equity vendors the `secp256k1` Rust crate which itself vendors `libsecp256k1`.

---

## 2. Current state (as of 2026-05-26)

Implemented:

| Crate | Purpose | Tests |
| --- | --- | --- |
| `utility` | `shorten_string` for debug output | 8 |
| `crypto` | SHA-1/256/512, RIPEMD-160, HMAC-SHA-512, PBKDF2-SHA-512, secp256k1 sign/verify, OS RNG | 19 |
| `p2p` | CompactSize varint, var-string, minimal `Message` envelope | 5 |
| `network` | Wire envelope (`Header`, `WireMessage`), 22 Bitcoin message types | 50 |
| `equity` | Base58 / Base58Check, keys, addresses, scripts, script engine (no sig check), transactions, blocks, merkle tree, difficulty target, BIP-39 mnemonics | 130 |

CLI apps: `bits`, `list-prefixes`, `view-transactions`. All three are convenience tools, not a node.

Not implemented (this document defines them):

- Persistent storage (UTXO set, block storage)
- Chain validator
- Mempool
- Peer connection manager and async I/O
- Address manager
- Headers-first sync and initial block download (IBD)
- RPC interface
- Configuration and logging frameworks
- The node binary itself

---

## 3. High-level architecture

### 3.1 Layered view

```text
+----------------------------------------------------------------------+
|                              APPLICATION                             |
| apps/node                                                            |
|   ├─ TCP listener / dialer       (tokio::net, sockets)               |
|   ├─ WireCodec                   (length-prefixed frames)            |
|   ├─ TokioTransport              (implements p2p::Transport)         |
|   └─ RPC HTTP server                                                 |
| apps/{bits,list-prefixes,view-tx}  (small CLI utilities)             |
+======================================================================+   <-- library / application boundary
|                                LIBRARIES                             |
|  sync         (headers-first IBD, block relay; consumes Transport)   |
|  mempool      (unconfirmed-tx pool)                                  |
|  peers        (per-peer state machine; pure logic, no sockets)       |
|  addrman      (peer address book, eviction; pure data structure)     |
|  validator    (consensus rules, block + tx checks)                   |
|  blockchain   (UTXO store, block store)                              |
|  equity       (consensus types: tx, block, script, target)           |
|  network      (P2P message types, WireMessage)                       |
|  p2p          (varint, var-string, **Transport trait**, NetEvent)    |
|  crypto       (hashes, ECC, RNG)                                     |
|  utility      (small helpers)                                        |
+----------------------------------------------------------------------+
```

The double-line band separates library crates from the application. **No library crate touches a socket.** Library logic interacts with the network only through the `p2p::Transport` trait (commands) and a `mpsc::Receiver<NetEvent>` (events). Both are defined in §4.14. The actual TCP implementation lives entirely in `apps/node`.

Lower layers are dependency-only; they do not call upward. The flow is data-down (received messages parsed and dispatched) and event-up (validator signals to sync, sync issues commands through the Transport trait to the app's network code).

### 3.2 Crate dependency graph (target end-state)

```text
                       === application ===
node ── (tokio, hyper) ──► rpc ─┐
  │                              ▼
  ├──────────────────────────► sync ──┬─► mempool ──┐
  │                                    │             │
  ├──► peers ─► addrman                 │             │
  │      └─────────────────► p2p ◄──────┴─► validator│
  │                           ▲                       │
  ▼                           │                       ▼
blockchain ──► equity ──► network ──► p2p ──► crypto ──► (none)
                                        └─► utility
                       === libraries ===
```

Notable edges:

- `node` is the **only** crate that pulls `tokio` and `tokio-util` from `[workspace.dependencies]`. Library crates may pull `futures-core` for type abstractions but **must not** depend on a specific async runtime.
- `peers` (pure-logic peer state machine) depends on `p2p`'s Transport trait and on `network` for the message types. It does not depend on `tokio`.
- `sync` depends on `p2p::Transport` to push commands and on a `mpsc::Receiver<NetEvent>` supplied by the app. The receiver type comes from the futures-core API, not Tokio specifically.
- `node` depends on every library crate and is the assembly point.

Rules:

- No back-edges. `equity` may not depend on `network`. `network` already depends on `equity` for `Block`/`Transaction` payload types; that is the only upward-looking edge below the storage layer and is acceptable because both crates share consensus types.
- **No library crate may open a TCP socket, bind a listener, spawn a Tokio task, or call `tokio::spawn`.** This is enforced by code review and (long-term) by a CI lint that greps for forbidden symbols in lib crates.
- `validator` depends on `blockchain` and `equity` but not on `peers` or `sync`. Validation is pure given the chain state.
- `sync` orchestrates `peers` + `validator` + `mempool` + `blockchain`. It is the highest-traffic crate.

### 3.3 Library / Application boundary

This is a project-wide design rule, not just a P2P-layer note:

> **Library crates contain no networking code.** All TCP I/O, async runtime usage, and connection lifecycle management lives in `apps/node`. The library expresses its needs through traits (commands to the network) and event channels (events from the network). The application implements the traits and produces the events.

Rationale:

1. **Testability.** Library logic can be tested with an in-memory fake `Transport` that records calls and a hand-fed event stream. No `tokio::test`, no port allocation, no fixture sockets.
2. **Runtime neutrality.** A consumer of the library may want to use a different async runtime, or no runtime (e.g. for fuzzing). The library compiles and runs without `tokio` available.
3. **Clear blast radius.** Network bugs are in one crate (`apps/node`). Consensus bugs are in another set (`equity`, `validator`, `blockchain`). Misbehavior reports cannot be ambiguously routed.
4. **Reusability.** A second binary (e.g. a future block-relay-only node, or a regression-test harness) reuses every library crate without inheriting `apps/node`'s policy choices.

The complete interface that crosses this boundary is specified in §4.14.

---

## 4. Layer-by-layer design

### 4.1 Crypto (`libs/crypto`) — implemented

Thin wrappers over `sha1`, `sha2`, `ripemd`, `hmac`, `pbkdf2`, `secp256k1`, `getrandom`. Fixed-size arrays as return types where the digest size is known. All wrappers are stateless free functions.

Public surface:

- `sha256::sha256`, `sha256::double_sha256`, `sha256::checksum`
- `sha512::sha512`
- `sha1::sha1`
- `ripemd::ripemd160`
- `hmac::hmac_sha512`
- `pbkdf2::pbkdf2_hmac_sha512`
- `ecc::{public_key_is_valid, private_key_is_valid, derive_public_key, sign, verify}`
- `random::get_bytes`

Notes:

- Fallible APIs at crate boundary use `anyhow::Result`. Per workspace policy.
- No constant-time helpers exposed; rely on the upstream crates.
- `secp256k1` global context is used (`SECP256K1`). Single-threaded contention is not a concern; the crate supports concurrent use.

### 4.2 P2P crate (`libs/p2p`) — partly implemented

The `p2p` crate has two responsibilities:

1. **Wire-format primitives** *(implemented)* — CompactSize varint, var-string, a minimal `Message { command, payload }` envelope. Deserializers return a result type carrying `(value, bytes_read)`; `bytes_read == 0` signals a short or malformed buffer. The newer cursor-style reader/writer functions in `libs/network::serialize` co-exist; consolidation tracked in §13.
2. **Library / application boundary** *(planned)* — the `p2p::transport` module defines the `Transport` trait, `NetEvent` enum, `PeerId`, `Direction`, `VersionInfo`, `DisconnectReason`, and `TransportError`. These are the only types that cross the library/application line. Full specification, examples, and rationale in §4.14.

Crate role summary:

| Module | Purpose |
| --- | --- |
| `p2p::varint`, `p2p::var_string` | Bitcoin CompactSize encodings (existing). |
| `p2p::message` | `Message { command, payload }` envelope (existing; predates `network::WireMessage`). |
| `p2p::transport` *(new)* | `Transport` trait, `NetEvent`, `PeerId`, related types. **No I/O.** Re-exports `network::messages::Message` for ergonomic consumer code. |

The `transport` module deliberately depends only on `std`, `thiserror`, `futures-core` (for the `Stream` trait used in event-side examples), and the `network` crate. It does **not** depend on `tokio`. This makes `Transport` implementable on any async runtime or none.

### 4.3 Consensus types (`libs/equity`) — implemented

Bitcoin core types and their wire/text encodings:

| Module | Type | Notes |
| --- | --- | --- |
| `base58`, `base58_check` | Free functions | Version byte is `u8`. SHA-256d checksum via `crypto::sha256::checksum`. |
| `private_key` | `PrivateKey` | 32-byte scalar + compressed flag. WIF and mini-key constructors. |
| `public_key` | `PublicKey` | Compressed or uncompressed. Validated via `secp256k1`. |
| `address` | `Address` | P2PKH only. `to_string(Network)` for prefix dispatch. |
| `script` | `Script` | Raw bytes + parsed `Instruction` list. `is_valid()` reflects parse success. |
| `instruction` | `Instruction`, `OpCode`, `Description` | 256-entry opcode table with min-stack-depth and validity. |
| `script_engine` | `ScriptEngine` | Stack interpreter. **Signature ops (`CHECKSIG` etc.) are intentional no-ops** for now. |
| `transaction` | `Transaction`, `Input`, `Output` | Version 1, non-segwit. |
| `txid` | `Txid` | Stored display-order; wire format is reversed. |
| `block` | `Block`, `BlockHeader` | 80-byte header + var-int transaction count + transactions. |
| `merkle_tree` | `MerkleTree` | Array-backed binary tree. Bitcoin odd-leaf duplication rule. |
| `target` | `Target` | nBits ↔ hash + difficulty. |
| `mnemonic` | `Mnemonic`, `generate_mnemonic`, `validate_mnemonic`, `mnemonic_to_seed` | BIP-39 English. Used today only by tests. Will be removed when the wallet module is excised. |
| `wallet` | `Wallet`, `WalletEntry` | **Will be removed / moved out.** Not part of the node. |
| `configuration` | `Configuration` | Compile-time mainnet defaults: `NETWORK`, `ADDRESS_VERSION`, `PRIVATE_KEY_VERSION`. |

Error type is `equity::EquityError`, a `String` wrapper. **Open question:** convert to `thiserror` with discriminable variants, or replace with `anyhow::Error` at boundaries. Current choice was made before the workspace standardized on `anyhow`.

### 4.4 Network messages (`libs/network`) — implemented

`WireMessage = Header + payload` where the 24-byte header carries magic, command name, payload length, and a 4-byte double-SHA-256 checksum. The 22 supported message variants live in `messages::Message`:

```text
Version, Verack, Addr, Inv, GetData, NotFound,
GetBlocks, GetHeaders, Headers, Block, Tx,
Ping, Pong, GetAddr, Reject,
FilterLoad, FilterAdd, FilterClear, MerkleBlock,
Alert, SendHeaders, Mempool
```

Each typed message provides `serialize(&mut Vec<u8>)`, `deserialize(&mut &[u8]) -> Result<Self>`, a `COMMAND: &'static str`, and (for diagnostics) `to_json()`.

Notable details:

- `VersionMessage` honors protocol version gates: fields after `to` only serialize when `version >= 106`; `relay` only when `version >= 70001`.
- `FilterLoad` enforces `MAX_FILTER_LOAD_SIZE = 36_000` and `MAX_FILTER_HASH_FUNCS = 50`.
- `FilterAdd` enforces `MAX_FILTER_ADD_SIZE = 520`.
- `Reject` consumes the rest of the stream as `data`.

`InventoryType` is a four-variant enum with `TryFrom<u32>` for wire decode.

### 4.5 Storage layer (`libs/blockchain`) — planned, see [blockchain-plan.md](blockchain-plan.md)

Two stores under one OS-correct data directory:

- **UTXO set** — RocksDB column family. Key `(txid, vout)` BE, value `(amount, script, height, coinbase)`. Bloom filter + LZ4 + prefix iterator.
- **Block bytes** — flat append-only `blk*.dat` files (Bitcoin Core convention). RocksDB column families index `block_hash → (file_num, offset, length, height, header)`, `height → block_hash`, and chain `meta`.

The header is duplicated into the RocksDB index so chain walks and locator construction need zero flat-file I/O.

Concurrency: `Arc<DB>`, RocksDB `Send+Sync`, writes batched per block, reads lock-free. Synchronous API; async callers wrap with `tokio::task::spawn_blocking`.

OS abstraction: file backend trait with cfg-gated impls for `unix` (`pread` / `fdatasync` / `F_FULLFSYNC`) and `windows` (`ReadFile`+`OVERLAPPED` / `FlushFileBuffers`).

See [blockchain-plan.md](blockchain-plan.md) for the full schema, phase order, and open questions.

### 4.6 Consensus validator (`libs/validator`) — planned

A pure module: given chain state and an incoming block or transaction, decide accept/reject and produce the state delta.

Two entry points:

```rust
pub fn check_block(block: &Block, ctx: &ValidationContext) -> Result<BlockDelta>;
pub fn check_transaction(tx: &Transaction, ctx: &MempoolContext) -> Result<TxDelta>;
```

Where `ValidationContext` carries: tip header, current target, median-time-past, UTXO snapshot reader, height. `BlockDelta` carries: new tip header, list of UTXO inserts/deletes, sigop count, block reward.

#### 4.6.1 Block-level checks (in order)

1. **Header sanity** — version > 0; merkle root within 32 bytes; timestamp not more than two hours in the future; PoW hash ≤ target.
2. **PoW** — `double_sha256(serialize(header)) ≤ target_hash(header.nBits)`.
3. **Difficulty rule** — `nBits` matches the retargeting schedule for this height (every 2016 blocks, ±4× clamp).
4. **Coinbase structure** — exactly one input with null prev-out and `0xffffffff` vout. Coinbase script length 2..=100. Block-reward output ≤ `subsidy(height) + sum(fees)`.
5. **Transaction loop** — for each non-coinbase tx call `check_transaction_in_block`.
6. **Merkle root** — recompute and compare.
7. **Sigop count** — ≤ `MAX_BLOCK_SIGOPS = 20_000`.

#### 4.6.2 Transaction-level checks (in order)

1. **Syntax** — non-empty inputs/outputs, no duplicate outpoints, output values in `[0, MAX_MONEY]`, sum of outputs ≤ sum of inputs.
2. **Standardness** (mempool-only, skipped during IBD) — script forms in known set, no dust outputs, max tx size.
3. **Input lookup** — every prev-out resolves to a UTXO and is not already spent in this block.
4. **Coinbase maturity** — if input spends a coinbase output, parent block must be at least 100 confirmations deep.
5. **Script verification** — for each input, run `scriptSig || scriptPubKey` through the script engine **with signature ops enabled**. This requires extending `script_engine` to compute the SIGHASH and call `crypto::ecc::verify`.
6. **nLockTime** — current height/time satisfies the lock.

#### 4.6.3 Soft fork bookkeeping

BIP-9 / BIP-8 deployment state machine is **out of scope for v0.1**. Constants (DERSIG height, BIP-65 / BIP-66 / BIP-112 / BIP-113) will be hard-coded by activation height for mainnet and testnet3.

### 4.7 Mempool (`libs/mempool`) — planned

In-memory pool of unconfirmed transactions.

Data:

```rust
struct Mempool {
    by_txid:    HashMap<Txid, Arc<MempoolEntry>>,
    by_outpoint: HashMap<OutPoint, Txid>,           // for double-spend detection
    by_fee_rate: BTreeMap<FeeRate, HashSet<Txid>>,  // for eviction + block template
    parents:    HashMap<Txid, HashSet<Txid>>,       // CPFP graph
    children:   HashMap<Txid, HashSet<Txid>>,
    bytes:      usize,
}
```

Operations:

- `add(tx)` — runs `validator::check_transaction` against the mempool context; updates the graph.
- `remove(txid, reason)` — on confirm, eviction, conflict, or RBF.
- `apply_block(block)` — bulk remove confirmed; re-evaluate dependents.
- `iter_for_block_template()` — descending fee rate, CPFP-aware.

Limits: `MAX_MEMPOOL_BYTES` default 300 MB. Below limit, no eviction; above, evict lowest fee rate until under.

**Open questions:** RBF (BIP-125) policy, package relay, full-RBF flag, ancestor/descendant limits. Defer until validator is solid.

### 4.8 Peer state machine (`libs/peers`) — planned

`libs/peers` is the pure-logic per-peer state machine. **It does not touch sockets.** It accepts decoded `Message` values and emits commands to be sent; the surrounding application is responsible for the bytes. This is the library half of the design decision documented in §3.3.

#### 4.8.1 Per-peer state machine

```text
Connecting ──► HandshakeSent ──► HandshakeReceived ──► Ready
                  │                  │                   │
                  └──────────────► Disconnecting ◄───────┘
```

- `Connecting` — TCP connect in progress (the *application* is in this state on behalf of the peer; the library is told once the socket is up via a `NetEvent::Connected`).
- `HandshakeSent` — local `version` sent, waiting for peer's `version` + `verack`.
- `HandshakeReceived` — peer's `version` arrived, `verack` sent; waiting for peer's `verack`.
- `Ready` — full duplex. Heartbeat via `ping`/`pong` every 2 minutes; idle timeout 90 minutes.
- `Disconnecting` — the library has decided to disconnect; the actual close is performed by the application after the library calls `Transport::disconnect`.

#### 4.8.2 Pure-logic shape

The `Peer` type is a plain struct, not a task:

```rust
pub struct Peer {
    id: PeerId,
    state: PeerState,
    banscore: u32,
    last_seen: Instant,
    pings: HashMap<u64, Instant>,
    // ...
}

impl Peer {
    /// Feed an inbound message. Returns the commands the application must execute.
    pub fn handle(&mut self, msg: Message, now: Instant) -> Vec<PeerAction>;

    /// Periodic tick to drive pings, timeouts. Returns commands.
    pub fn tick(&mut self, now: Instant) -> Vec<PeerAction>;
}

pub enum PeerAction {
    Send(Message),
    Disconnect(&'static str),
    Ban(Duration),
    Emit(NetEvent),  // upward event toward sync
}
```

This shape is trivially unit-testable with no runtime. The application wires it to real I/O.

#### 4.8.3 Peer registry (in `apps/node`)

The peer manager lives in `apps/node`, not in `libs/peers`. It owns:

- The Tokio listener and dialer.
- A `HashMap<PeerId, PeerTask>` where each `PeerTask` is the reader/writer/logic task trio for one TCP connection.
- The `TokioTransport` value passed to the library logic so that `Peer::handle` → `PeerAction::Send` ultimately reaches the socket.

Async architecture (one task per peer, reader/writer split) and outbound slot maintenance are detailed in [p2p-design.md](p2p-design.md) §3.2, §7.

#### 4.8.4 Banscore

Each `Peer` carries a misbehavior score; the library's `PeerAction::Ban(duration)` is the only way it asks for a ban. The application maintains the persistent `banlist.json`. Reaching 100 triggers a 24-hour ban. **Open question:** whether to also evict on slow message rate or only on protocol violations.

### 4.9 Address manager (`libs/addrman`) — planned

Tracks known peer addresses across restarts.

Data model adapted from Bitcoin Core's `CAddrMan`:

- **New table** — addresses heard about but not yet successfully connected to. Bucketed by source-IP group.
- **Tried table** — addresses we have successfully connected to. Bucketed by the address's own IP group.

Selection biased toward recent successes. Persists as `peers.dat` (custom format, **not** Bitcoin Core compatible) in the data dir.

**Open question:** whether to support DNS seeds and which set. Bitcoin Core's are the obvious starting point.

### 4.10 Sync orchestrator (`libs/sync`) — planned

The brain. Subscribes to peer events, drives chain progress.

#### 4.10.1 Headers-first IBD

1. On peer ready, send `getheaders` with our locator (sparse list from tip back to genesis: heights `tip, tip-1, tip-2, tip-4, tip-8, ..., 0`).
2. Process `headers` responses, validate header chain (PoW + difficulty), write to blockchain header index.
3. Once headers reach a sufficient height (peer's reported best, or within `MAX_HEADERS_DRIFT = 1`), switch to block download.
4. Block download: maintain a sliding window of in-flight `getdata`s, one peer assigned per range. Stall detection: if a peer doesn't deliver within 10 s, reassign.
5. Apply blocks in order via `validator::check_block`. On accept, commit UTXO delta and block bytes atomically.

#### 4.10.2 Steady-state

After IBD:

- Listen for `inv` messages, request unknown items via `getdata`.
- Relay accepted blocks and txs via `inv` to other peers.
- Compact-block relay (BIP-152) is a follow-up, not v0.1.

#### 4.10.3 Reorg

When a block extends a sibling chain whose accumulated work exceeds the current tip:

1. Walk back to common ancestor.
2. Undo each block on the old branch using undo data (see [blockchain-plan.md](blockchain-plan.md) open questions).
3. Apply new branch blocks.

Reorg support is gated on undo data, which is a deferred phase in the blockchain crate plan.

### 4.11 RPC (`libs/rpc`) — planned

Minimal JSON-RPC 2.0 server over HTTP. Method set, initial cut:

| Method | Purpose |
| --- | --- |
| `getblockchaininfo` | tip hash, height, difficulty, headers count, IBD flag |
| `getblockhash <h>` | hash for height |
| `getblock <hash>` | full block JSON |
| `getrawtransaction <txid>` | tx hex (mempool + blocks if `txindex`) |
| `sendrawtransaction <hex>` | submit to mempool |
| `getmempoolinfo` | size, bytes, fee histogram |
| `getpeerinfo` | per-peer state |
| `stop` | clean shutdown |

Auth: cookie file in data dir (`.cookie`), HTTP basic-auth header. Same convention as Bitcoin Core. **No wallet RPCs.**

### 4.12 Configuration (`libs/config` or in node binary) — planned

CLI flags via `clap`, optional `equity.conf` file (TOML). Resolution order: CLI > env > file > defaults. Settings:

| Key | Default | Description |
| --- | --- | --- |
| `network` | `mainnet` | mainnet / testnet3 / regtest |
| `datadir` | OS default | overrides `directories::ProjectDirs` |
| `listen` | `true` | accept inbound connections |
| `port` | `8333` | TCP port (network-dependent default) |
| `max_connections` | `125` | inbound + outbound cap |
| `outbound_target` | `8` | desired outbound peer count |
| `dbcache` | `450` MB | RocksDB block cache |
| `rpc_bind` | `127.0.0.1` | RPC listener |
| `rpc_port` | `8332` | |
| `log_level` | `info` | trace / debug / info / warn / error |
| `prune` | `0` | 0 = archival; >0 = MB target |

### 4.13 Node binary (`apps/node`) — planned

Wires everything: parse config, open `Database`, start `addrman`, start the listener, dial outbound peers, construct a `TokioTransport`, spawn the sync task with the transport handle, start the RPC server, install Ctrl-C handler for graceful shutdown. Drop order on shutdown is the reverse of startup.

`apps/node` is the **only** crate that links `tokio`, `tokio-util`, `hyper`, and `directories`. Everything below the application line uses synchronous, runtime-neutral types.

### 4.14 P2P transport interface (`p2p::transport`) — planned

The library / application boundary in §3.3 has exactly two crossings:

1. **Commands** flow from library to application through the `Transport` trait (a normal Rust trait, synchronous methods, runtime-neutral).
2. **Events** flow from application to library through a `futures::channel::mpsc::Receiver<NetEvent>` (or `tokio::sync::mpsc::Receiver` in the `apps/node` case — both implement `futures::Stream`, so the consumer is generic).

This whole interface lives in `libs/p2p` under `p2p::transport`. The `network` crate's `Message` type is re-exported here so library consumers do not need to depend on `network` directly.

#### 4.14.1 Types

```rust
//! libs/p2p/src/transport.rs
//!
//! Interface between the consensus and sync logic (library crates) and
//! the network I/O implementation (the node application). The library
//! never touches sockets; it talks to the network only through this
//! module.

use std::net::SocketAddr;
use std::time::Duration;
use thiserror::Error;

pub use network::Message;  // re-exported for convenience

/// Opaque per-connection identifier minted by the Transport implementer.
/// Monotonic within one process; never reused.
#[derive(Debug, Clone, Copy, Hash, PartialEq, Eq, PartialOrd, Ord)]
pub struct PeerId(pub u64);

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Direction {
    Inbound,
    Outbound,
}

#[derive(Debug, Clone)]
pub struct VersionInfo {
    pub protocol_version: u32,
    pub services: u64,
    pub user_agent: String,
    pub height: u32,
    pub relay: bool,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DisconnectReason {
    PeerClosed,
    Idle,
    PingTimeout,
    HandshakeTimeout,
    BadFrame,
    BadMessage,
    Banned,
    DialFailed,
    SelfConnect,
    Eviction,
    LocalShutdown,
}

/// Events the application pushes to the library through a channel.
#[derive(Debug, Clone)]
pub enum NetEvent {
    /// A TCP connection has been established. Handshake has not yet completed.
    Connected {
        peer: PeerId,
        addr: SocketAddr,
        direction: Direction,
    },
    /// Handshake (`version` ↔ `verack`) finished; full duplex is open.
    Ready { peer: PeerId, info: VersionInfo },
    /// A decoded message arrived from the peer.
    Message { peer: PeerId, msg: Message },
    /// The connection has been torn down (either side, for any reason).
    Disconnected {
        peer: PeerId,
        reason: DisconnectReason,
    },
}

#[derive(Debug, Error)]
pub enum TransportError {
    #[error("peer no longer connected")]
    PeerGone,
    #[error("outbound queue is full; backpressure")]
    Backpressure,
    #[error("transport is shutting down")]
    Shutdown,
}

/// Commands the library issues to the application.
///
/// Implementations live in `apps/node` (typically a thin wrapper around
/// per-peer `tokio::sync::mpsc::Sender<Message>` handles).
///
/// All methods are synchronous and non-blocking. None of them perform
/// I/O on the calling thread; they enqueue work for the per-peer writer
/// task to drain.
pub trait Transport: Send + Sync + 'static {
    /// Send `msg` to a specific peer. Returns immediately.
    ///
    /// - `Err(PeerGone)` — the peer disconnected or was never known.
    /// - `Err(Backpressure)` — the per-peer outbound queue is full.
    ///   The caller chooses whether to drop, retry, or escalate.
    fn send(&self, peer: PeerId, msg: Message) -> Result<(), TransportError>;

    /// Send `msg` to **every** connected peer that has reached the
    /// `Ready` state. Returns the number of peers that accepted the
    /// message into their queue. Peers experiencing backpressure are
    /// silently skipped.
    fn broadcast(&self, msg: Message) -> usize;

    /// Disconnect `peer` with a textual reason (logged, not sent on the wire).
    /// Idempotent; calling on an unknown peer is a no-op.
    fn disconnect(&self, peer: PeerId, reason: &'static str);

    /// Disconnect `peer` and refuse new connections from its IP for `duration`.
    /// Idempotent; safe to call repeatedly.
    fn ban(&self, peer: PeerId, duration: Duration);
}
```

#### 4.14.2 Ownership and lifetime

The application creates exactly one `Transport` value at startup and shares it via `Arc<dyn Transport>` with every library consumer that needs to issue commands. The corresponding `NetEvent` channel has exactly one consumer — typically the `sync` task — which fans out events internally if multiple subsystems care about them.

```text
                            ┌──────────────────────┐
                            │  apps/node           │
                            │   TokioTransport ◄───┼─── Arc<dyn Transport>  (commands)
                            │     │                │
   sockets ◄── reader/writer ─────┤                │
                            │     │                │
                            │     ▼                │
                            │   events_tx ────────►│─── mpsc::Receiver<NetEvent>  (events)
                            └──────────────────────┘                 │
                                                                     ▼
                                                            ┌────────────────┐
                                                            │ libs/sync      │
                                                            │  consumes      │
                                                            │  events,       │
                                                            │  calls send()  │
                                                            └────────────────┘
```

#### 4.14.3 Threading and async neutrality

The trait is **runtime-neutral**:

- Methods are synchronous. `send` does not `.await`; it pushes onto a bounded `mpsc::Sender` via `try_send`.
- The trait is `Send + Sync + 'static`. Implementations may freely share via `Arc`.
- Returning errors instead of awaiting backpressure puts the policy decision (drop / retry / disconnect) in the library, where it belongs.

The event side uses a channel rather than a callback because:

- The library wants a single point to `select!` events against timers and other inputs.
- Channels keep the application free to choose its backpressure policy on the event side too (if the library is slow, `Sender::send().await` in the application naturally stalls socket reads).

#### 4.14.4 Example — library side (a tiny header-sync consumer)

```rust
use std::sync::Arc;
use std::time::Duration;
use anyhow::Result;
use p2p::transport::{NetEvent, PeerId, Transport};
use p2p::Message;
use network::messages::{LocatorMessage, GetHeadersMessage};

/// Drive header sync against whichever peers the application provides.
///
/// `transport`: shared handle the application supplied at startup.
/// `events`:    receiver of NetEvents pushed by the application.
pub async fn run_header_sync<R>(
    transport: Arc<dyn Transport>,
    mut events: R,
) -> Result<()>
where
    R: futures::Stream<Item = NetEvent> + Unpin,
{
    use futures::StreamExt;

    while let Some(event) = events.next().await {
        match event {
            NetEvent::Ready { peer, info } => {
                tracing::info!(?peer, height = info.height, "peer ready");
                let getheaders = GetHeadersMessage(LocatorMessage {
                    version: 70015,
                    hashes: vec![[0u8; 32]],   // genesis-only locator for brevity
                    last:   [0u8; 32],
                });
                // Library calls into the application. Synchronous, non-blocking.
                if let Err(e) = transport.send(peer, Message::GetHeaders(getheaders)) {
                    tracing::warn!(?peer, error = %e, "failed to send getheaders");
                }
            }
            NetEvent::Message {
                peer,
                msg: Message::Headers(h),
            } => {
                if !validate_headers(&h.blocks) {
                    transport.ban(peer, Duration::from_secs(86_400));
                    continue;
                }
                store_headers(&h.blocks);
            }
            NetEvent::Disconnected { peer, reason } => {
                tracing::info!(?peer, ?reason, "peer disconnected");
            }
            _ => {}
        }
    }
    Ok(())
}

fn validate_headers(_: &[equity::block::Block]) -> bool { /* ... */ true }
fn store_headers(_: &[equity::block::Block]) { /* ... */ }
```

Note: the function takes any `Stream<Item = NetEvent>`. In tests we pass `futures::stream::iter([...])`; in production the application passes a `tokio::sync::mpsc::Receiver` (which implements `Stream` via `tokio_stream::wrappers::ReceiverStream`).

#### 4.14.5 Example — application side (`TokioTransport` skeleton)

```rust
use std::collections::HashMap;
use std::net::SocketAddr;
use std::sync::Arc;
use std::time::Duration;

use parking_lot::RwLock;
use tokio::sync::mpsc;

use p2p::transport::{
    Direction, DisconnectReason, NetEvent, PeerId, Transport, TransportError, VersionInfo,
};
use p2p::Message;

/// Lives in `apps/node/src/transport.rs`. The library never sees this type
/// directly; it only sees `Arc<dyn Transport>`.
pub struct TokioTransport {
    inner: Arc<Inner>,
}

struct Inner {
    /// One bounded sender per connected peer; the receiver lives in the
    /// per-peer writer task that owns the socket's write half.
    peers: RwLock<HashMap<PeerId, PeerHandle>>,
    /// Sent to the library so it can observe lifecycle events.
    events_tx: mpsc::Sender<NetEvent>,
    /// Persistent IP-level ban list.
    banlist: Arc<crate::ban::BanList>,
}

struct PeerHandle {
    addr: SocketAddr,
    direction: Direction,
    out_tx: mpsc::Sender<Message>,
}

impl TokioTransport {
    pub fn new(
        events_tx: mpsc::Sender<NetEvent>,
        banlist: Arc<crate::ban::BanList>,
    ) -> Arc<Self> {
        Arc::new(Self {
            inner: Arc::new(Inner {
                peers: RwLock::new(HashMap::new()),
                events_tx,
                banlist,
            }),
        })
    }

    /// Called by the accept/dial loops after a socket is up.
    pub fn register(&self, peer: PeerId, addr: SocketAddr, dir: Direction, out_tx: mpsc::Sender<Message>) {
        self.inner.peers.write().insert(peer, PeerHandle {
            addr,
            direction: dir,
            out_tx,
        });
        let _ = self.inner.events_tx.try_send(NetEvent::Connected {
            peer,
            addr,
            direction: dir,
        });
    }

    /// Called by the per-peer reader task when the socket closes.
    pub fn drop_peer(&self, peer: PeerId, reason: DisconnectReason) {
        self.inner.peers.write().remove(&peer);
        let _ = self.inner.events_tx.try_send(NetEvent::Disconnected { peer, reason });
    }
}

impl Transport for TokioTransport {
    fn send(&self, peer: PeerId, msg: Message) -> Result<(), TransportError> {
        let peers = self.inner.peers.read();
        let handle = peers.get(&peer).ok_or(TransportError::PeerGone)?;
        match handle.out_tx.try_send(msg) {
            Ok(()) => Ok(()),
            Err(mpsc::error::TrySendError::Full(_)) => Err(TransportError::Backpressure),
            Err(mpsc::error::TrySendError::Closed(_)) => Err(TransportError::PeerGone),
        }
    }

    fn broadcast(&self, msg: Message) -> usize {
        let peers = self.inner.peers.read();
        peers
            .values()
            .filter(|h| h.out_tx.try_send(msg.clone()).is_ok())
            .count()
    }

    fn disconnect(&self, peer: PeerId, reason: &'static str) {
        tracing::info!(?peer, %reason, "disconnect requested by library");
        // Removing the sender drops the per-peer writer task's mpsc::Receiver,
        // which causes the writer to exit; its cancellation token then closes
        // the reader, and the reader emits Disconnected.
        self.inner.peers.write().remove(&peer);
    }

    fn ban(&self, peer: PeerId, duration: Duration) {
        if let Some(handle) = self.inner.peers.write().remove(&peer) {
            self.inner.banlist.ban(handle.addr.ip(), duration);
        }
    }
}
```

The per-peer reader task (not shown) decodes frames via `WireCodec` and pushes each decoded `Message` into `events_tx` as `NetEvent::Message { peer, msg }`. The matching writer task drains `out_tx` and writes to the socket. Both are owned by `apps/node`, never by a library crate.

#### 4.14.6 Example — testing a library consumer

Because `Transport` is a trait and `NetEvent` is a plain enum, no async runtime is needed to test logic that consumes them:

```rust
use std::sync::{Arc, Mutex};
use p2p::transport::{NetEvent, PeerId, Transport, TransportError};
use p2p::Message;

#[derive(Default)]
struct FakeTransport {
    sent: Mutex<Vec<(PeerId, Message)>>,
    banned: Mutex<Vec<PeerId>>,
}

impl Transport for FakeTransport {
    fn send(&self, peer: PeerId, msg: Message) -> Result<(), TransportError> {
        self.sent.lock().unwrap().push((peer, msg));
        Ok(())
    }
    fn broadcast(&self, _msg: Message) -> usize { 0 }
    fn disconnect(&self, _peer: PeerId, _reason: &'static str) {}
    fn ban(&self, peer: PeerId, _duration: std::time::Duration) {
        self.banned.lock().unwrap().push(peer);
    }
}

#[test]
fn header_sync_sends_getheaders_on_ready() {
    let fake = Arc::new(FakeTransport::default());
    let events = futures::stream::iter([NetEvent::Ready {
        peer: PeerId(1),
        info: dummy_version_info(),
    }]);
    futures::executor::block_on(run_header_sync(fake.clone(), events)).unwrap();
    let sent = fake.sent.lock().unwrap();
    assert_eq!(sent.len(), 1);
    assert!(matches!(sent[0].1, Message::GetHeaders(_)));
}
```

No `tokio::test`, no port allocation, no fixture sockets — the test exercises only library logic.

#### 4.14.7 Why a trait and not a concrete type

A concrete `Transport` struct would force every library to link `tokio` transitively. The trait keeps three things separable:

- The **library** depends only on the trait and `Message`.
- The **application** picks an implementation.
- The **tests** mock the trait.

Other implementations are possible without library changes: an in-process pipe transport for two-node integration tests; a recording transport for capture-and-replay; a future `quictransport` for BIP-324 layered over QUIC.

---

## 5. Concurrency model

### 5.1 Runtime

- Single multi-threaded Tokio runtime (`#[tokio::main(flavor = "multi_thread")]`).
- Worker thread count = `num_cpus::get()` by default.
- RocksDB calls are synchronous syscalls; they go through `tokio::task::spawn_blocking`. The blocking pool is sized separately.

### 5.2 Shared state

- `Arc<Database>` is the canonical handle for storage. Cheaply cloneable.
- `Arc<Mempool>` wraps a `parking_lot::RwLock<MempoolInner>`. Readers (block template, RPC) take the read lock; writers (validator, sync) take the write lock briefly.
- `Arc<AddrMan>` similar.
- Peer-to-sync events flow via `tokio::sync::mpsc`. Sync-to-peer commands via per-peer `mpsc`.

### 5.3 Backpressure

- Peer inbox is bounded (capacity 128). Slow consumer = disconnect, not deadlock.
- Block download dispatcher tracks in-flight requests; never issues more than `MAX_INFLIGHT_BLOCKS_PER_PEER = 16`.

### 5.4 What we do NOT do

- No locks across `.await`. The `parking_lot::RwLock` is sync-only; if held across an await it would block the worker.
- No `unsafe`. The previous `offset_from` ptr math in `script.rs` was removed.
- No global mutable state beyond the secp256k1 context.

---

## 6. Error handling

### 6.1 Strategy

Two layers:

1. **Library APIs** return `anyhow::Result<T>` at crate boundaries. This is the workspace convention (see [CLAUDE.md](../CLAUDE.md)).
2. **Internal validator** uses a discriminable enum so the sync layer can decide whether to ban the peer:

```rust
#[derive(Debug, thiserror::Error)]
pub enum ConsensusError {
    #[error("bad PoW")]          BadPow,
    #[error("bad merkle root")]  BadMerkle,
    #[error("bad signature")]    BadSignature,
    #[error("bad coinbase")]     BadCoinbase,
    /* ... */
}
```

Mapping: a peer that sends a `ConsensusError`-flavored block gets banscore +100 (instant ban). A peer that sends malformed wire bytes gets +10.

### 6.2 Panics

Panics indicate bugs, not bad input. Anything coming off the wire must be handled by `Result`; anything coming from internal invariants may panic. Specifically:

- `unwrap()` is fine in tests and inside code that just `try_into()`'d a slice of known length.
- `unreachable!()` for genuinely-impossible match arms.
- No `panic!()` in production code paths.

---

## 7. Logging and metrics

### 7.1 Logging

`tracing` + `tracing-subscriber`. Three levels in production:

- `info` — startup, shutdown, peer connect/disconnect, new tip, reorg.
- `debug` — per-message events at peer scope.
- `trace` — per-script-op events, full hex dumps.

Per-module filter via `RUST_LOG`. No `println!`, no `eprintln!` outside the CLI apps' `main`.

### 7.2 Metrics

Eventually a Prometheus exporter on a separate port. Counters: messages by command, blocks accepted, blocks rejected by reason, mempool size in bytes/txs, peers by state. Gauges: tip height, header height, IBD progress. Histograms: block validation time, script verify time. Out of scope for v0.1, but APIs should be designed so the `metrics` crate can drop in without rework.

---

## 8. Security considerations

### 8.1 Untrusted input

Every byte from the network is hostile until validated. Specifically:

- `varint` decoders cap at sane sizes before allocating.
- `read_var_bytes` must not allocate based on attacker-controlled length without checking against the remaining buffer.
- Script execution has a stack-depth cap (`MAX_STACK_DEPTH = 1000`) and per-op cost ceiling.
- Block size cap before deserialization (`MAX_BLOCK_SERIALIZED_SIZE = 1_000_000` pre-SegWit).

### 8.2 DoS surfaces

- **Connection exhaustion** — bounded inbound count, source-IP rate limit on `version`.
- **Memory exhaustion** — bounded mempool, bounded inflight downloads, bounded per-peer inbox.
- **CPU exhaustion** — script-verify cap, parallel verification across peer messages.

### 8.3 Side channels

Not in scope (no signing in the node). When/if a signer is ever added, it goes in a separate crate with constant-time primitives.

### 8.4 Supply chain

Pinned dependency versions in `Cargo.toml`. `cargo audit` in CI. No `git` or `path` dependencies outside the workspace.

---

## 9. Testing strategy

### 9.1 Unit tests

Per module, inline `#[cfg(test)] mod tests`. Already convention in the workspace.

### 9.2 Property tests

`proptest`. Already used for hex round-trip, varint round-trip, Base58Check round-trip, merkle proofs, public-key serialization. Extend to: block round-trip, transaction signature verification, mempool ordering.

### 9.3 Vector tests

External test vectors:

- BIP-39 vectors — done.
- Bitcoin Core script test JSON (`script_tests.json`) — pending. Will live under `libs/equity/tests/vectors/`.
- Bitcoin Core tx test JSON (`tx_valid.json`, `tx_invalid.json`) — pending.

### 9.4 Integration tests

Per-crate `tests/` directories. Multi-crate scenarios live under a top-level `tests/` (to be created). Notable scenarios:

1. **Two-node handshake.** Spawn two in-process nodes connected via a `tokio::io::duplex()` pipe. Verify `version`/`verack`.
2. **Headers sync from a canned chain.** Pre-built 100-block chain fixture; one node serves, one syncs.
3. **Reorg.** Two-chain fixture; node sees both, picks heavier.
4. **Mempool acceptance + relay.** Inject tx, verify peer-2 receives `inv`.

### 9.5 Fuzzing

`cargo-fuzz` targets for the parsers: `Transaction::deserialize`, `Block::deserialize`, `WireMessage::from_bytes`, `Script::from_data`, varint, var-string. CI runs short fuzzes; longer corpora live offline.

---

## 10. Build and platforms

### 10.1 Toolchain

- Rust stable, edition 2024 (1.85+).
- No nightly features.
- MSVC on Windows; default linker on Linux/macOS.

### 10.2 Vendored C dependencies

- `secp256k1-sys` vendors `libsecp256k1`. No system `libsecp256k1` required.
- `librocksdb-sys` (after the blockchain crate lands) vendors RocksDB. Long first build; default features pruned to `snappy`+`lz4`+`multi-threaded-cf` to keep build time tolerable.

### 10.3 Cargo target dir

`D:/build/rust/target` per `.cargo/config.toml`. Override with `CARGO_TARGET_DIR` for non-default checkouts. CI uses the default.

### 10.4 Cross-OS quirks

- File paths via `directories::ProjectDirs`.
- File I/O backend for the flat block files is cfg-gated (`unix` vs `windows`).
- `fdatasync` vs `F_FULLFSYNC` vs `FlushFileBuffers` selected at compile time.

---

## 11. Reference implementations consulted

Where this document had to invent something, the choice is grounded in one of:

- **Bitcoin Core** (C++) — the canonical reference. Headers-first sync, `CAddrMan`, `blk*.dat` storage layout, banscore semantics, JSON-RPC method set.
- **btcd** (Go) — peer state machine and async I/O patterns. Good model for a separate reader/writer task per peer.
- **libbitcoin** (C++) — modular layer split. Inspired the layered diagram in §3.1.
- **Floresta** (Rust) — utreexo-based pruned node. Reviewed for crate organization in a Rust context; we do **not** adopt utreexo for v0.1.
- **rust-bitcoin** — primitive types (script, transaction). We have intentionally re-implemented rather than depend on it, because Equity is a learning exercise; but the rust-bitcoin API shapes informed ours.

### 11.1 Authoritative protocol references

When this document, the code, and the BIPs disagree, the BIPs win for consensus rules and wire format. Use these as primary sources, in this order:

1. [`github.com/bitcoin/bips`](https://github.com/bitcoin/bips) — authoritative BIP repository. Cite BIPs by number and link to the file in this repository (e.g. `bips/bip-0152.mediawiki`).
2. [Bitcoin developer guide](https://developer.bitcoin.org/devguide/) — readable narrative covering the same material at a higher level. The [P2P network](https://developer.bitcoin.org/devguide/p2p_network.html) page is the most relevant for this project; the IBD, block-relay, and banscore sections are summarized in [p2p-design.md](p2p-design.md).
3. [Bitcoin developer reference](https://developer.bitcoin.org/reference/) — message layouts, RPC schema, opcode tables.
4. Bitcoin Core source — last resort when a detail is in none of the above. Treat as informative, not normative.

---

## 12. Roadmap (suggested phase order)

1. **Storage** — implement `libs/blockchain` per [blockchain-plan.md](blockchain-plan.md).
2. **Validator** — `libs/validator` with block + tx checks, signature verification wired into the script engine.
3. **Remove wallet** — drop `equity::wallet` and `equity::mnemonic`, or move them to a separate `tools` crate.
4. **Peers + addrman** — async connection manager, persistent address book.
5. **Sync** — headers-first IBD, block download, reorg (gated on undo data).
6. **Mempool** — acceptance, eviction, relay.
7. **RPC** — minimal method set, cookie auth.
8. **Node binary** — `apps/node` wires it all.
9. **SegWit / Taproot** — separate document; not in v0.1.

---

## 13. Open questions (project-wide)

Tracked here so they are not forgotten. Each blocks at least one phase of the roadmap.

1. **Wallet module fate.** Delete entirely, move to a `tools` crate, or keep as a learning aid?
2. **`equity::EquityError` future.** Convert to `thiserror`, switch to `anyhow`, or leave?
3. **`p2p` vs `network::serialize`.** Consolidate or keep both?
4. **DNS seeds.** Which set? Are we comfortable hard-coding Bitcoin Core's?
5. **Pruning.** Archival only for v0.1, or design pruning from the start?
6. **Undo data format.** Bitcoin Core uses `rev*.dat`; we have not specified ours.
7. **RBF policy.** BIP-125 opt-in, full-RBF, or none?
8. **BIP-9/BIP-8.** Hard-code soft-fork activations for mainnet/testnet3, or implement the deployment state machine?
9. **Compact block relay (BIP-152).** v0.1 or follow-up?
10. **SegWit cutover.** Bolt onto existing transaction type, or introduce a parallel witness-aware type?
11. **Lib/app boundary enforcement.** Should CI add a lint (e.g. cargo-deny `bans.deny` for `tokio` in lib crates, or a custom check) that fails if a library crate gains a `tokio` import? Manual review alone is fragile.
12. **Runtime-neutral event channel type.** Should the `NetEvent` channel in `p2p::transport` expose `futures::channel::mpsc` or just require any `Stream`? Current draft is `Stream`-generic — verify that the resulting Tokio bridge does not require `tokio-stream` in any lib crate.

---

## 14. How to use this document

### 14.1 For human contributors

- §3 gives the layered picture. Start there.
- §4 is the layer-by-layer reference. Each subsection labels its implementation status.
- §12 is the roadmap; pick the next unblocked item.
- §13 is the parking lot. If you start on an item that depends on one of these, resolve the question first and update this file.

### 14.2 For automated agents

- This file plus [blockchain-plan.md](blockchain-plan.md) and [../CLAUDE.md](../CLAUDE.md) together form the complete spec for a cold-start agent. Read all three before writing code.
- Sections marked "implemented" describe code that exists on `develop`. Verify by reading the cited file before relying on a detail; this document may lag behind the code.
- Sections marked "planned" describe target state. If asked to implement, follow the design here unless an open question blocks the choice.
- Sections labelled "open question" are gates. Resolve before writing code that depends on the answer.
- When this document and the code disagree, the code is authoritative for current behavior; this document is authoritative for intended direction.

---

## 15. Change log

| Date | Change | Author |
| --- | --- | --- |
| 2026-05-26 | Initial draft. | Claude (Opus 4.7), session-authored |
| 2026-05-26 | Added authoritative protocol references (BIPs repo, devguide). | Claude (Opus 4.7), session-authored |
| 2026-05-26 | Recorded library/application boundary decision: no library crate touches a socket. Updated §3.1 layered diagram, §3.2 dep graph, added §3.3 boundary rule, rewrote §4.2 (p2p crate gains `transport` module), rewrote §4.8 (peer FSM is now pure logic), added §4.14 transport interface with `Transport` trait, `NetEvent` enum, and three worked examples (library consumer, `TokioTransport` skeleton, fake-transport test). | Claude (Opus 4.7), session-authored |
