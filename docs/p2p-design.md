# Equity — P2P Network Layer Design

Version: 0.1 (draft)
Status: design, not implemented.
Companion to [technical-design.md](technical-design.md) (§4.8 peer manager, §4.9 addrman, §4.10 sync) and [blockchain-plan.md](blockchain-plan.md).

This document specifies the network/peer subsystem: how Equity opens TCP connections to Bitcoin peers, frames bytes into messages, drives the handshake, maintains a population of peers, defends against misbehavior, and hands typed events to the sync orchestrator.

It does **not** specify chain validation, block download scheduling, or mempool logic — those live in `sync` / `validator` / `mempool`. The network layer is intentionally dumb about chain semantics; it speaks bytes, not consensus.

---

## 1. Scope and responsibilities

### 1.1 In scope

1. TCP transport. IPv4 and IPv6. No TLS, no Tor in v0.1 (see §15.3).
2. Length-prefixed framing of `WireMessage` envelopes on top of the byte stream.
3. Per-peer finite state machine for the handshake.
4. Per-peer keep-alive (`ping`/`pong`) and idle timeout.
5. Connection lifecycle: dial, accept, drop, ban.
6. Outbound connection slot management driven by address manager.
7. Inbound listener with per-source-IP rate limiting.
8. Banscore tracking, persistent banlist, eviction policy.
9. Address book (`addrman`): learning addresses via `addr`, persisting `peers.dat`.
10. Plumbing of decoded `Message` events upward and serializing outbound `Message` commands downward.

### 1.2 Out of scope (handled elsewhere)

- Header chain validation, block validation, UTXO updates (`validator`).
- IBD progress, block-download scheduling, reorg (`sync`).
- Mempool admission, tx relay decisions (`mempool` + `sync`).
- RPC.
- Compact block relay (BIP-152), block-relay-only peers, anchor connections — follow-ups.

### 1.3 Crate placement

The network/peer subsystem spans three crates:

| Crate | Role |
| --- | --- |
| `network` (exists) | Pure data types and codec — `Header`, `WireMessage`, `Message`, `Address`, `InventoryId`. **No I/O.** |
| `addrman` (new) | Address book. No I/O on the hot path; persistence behind a small `Persister` trait. |
| `peers` (new) | Async I/O, connection lifecycle, per-peer state machine, peer manager. Depends on `network`, `addrman`, `crypto` (for `random::get_bytes`). |

`sync` consumes `peers` and `addrman` via async channels. `peers` never calls into `sync` directly.

---

## 2. Layered view

```text
+--------------------------------------------------------------+
|  sync   (consumes typed events, issues typed commands)       |
+--------------------------------------------------------------+
|  PeerManager  (listener, dialer, peer registry)              |
|     ▲                                                        |
|     │  events: Connected, Ready, Message, Disconnected       |
|     │  commands: Send, Ban, Disconnect                       |
|     ▼                                                        |
|  Peer  (per-connection task trio: reader, writer, logic)     |
+--------------------------------------------------------------+
|  WireCodec   (length-prefixed Header + payload framing)      |
+--------------------------------------------------------------+
|  tokio::net::TcpStream / TcpListener                         |
+--------------------------------------------------------------+
```

Data flow:

- **Inbound**: bytes → `WireCodec::decode` → `WireMessage` → `Message::deserialize_payload` → `PeerEvent::Message` → manager → `sync`.
- **Outbound**: `sync` → `PeerCommand::Send(msg)` → manager → per-peer writer → `WireMessage::encode` → bytes.

Errors at the codec or message layer never reach `sync` as a panic; they become `PeerEvent::Banscore` deltas or a `PeerEvent::Disconnected` with a reason.

---

## 3. Runtime model

### 3.1 Tokio

One multi-threaded Tokio runtime, owned by the node binary. The peer manager and every peer task spawn on it. No separate runtime, no `block_on` islands.

### 3.2 Tasks per peer

Three tasks per peer, joined when any one exits:

| Task | Owns | Awaits on |
| --- | --- | --- |
| **reader** | read half of socket, decoder state | socket readable |
| **writer** | write half of socket | outbound `mpsc::Receiver<Message>` |
| **logic** | state machine, timers, banscore | inbound `mpsc::Receiver<RawMessage>`, ticker |

The reader/writer split prevents a slow remote from blocking our reads of its own messages or of unrelated peers. The logic task is the only place that mutates per-peer state, eliminating intra-peer locks.

```text
   socket ─────┐                      ┌──── socket
               ▼                      ▲
            reader ──mpsc──► logic ──mpsc──► writer
                                │
                                ▼
                        manager event bus
```

### 3.3 Channels

- **reader → logic**: `mpsc::channel(128)`. Bounded. If full, reader awaits — natural backpressure on a peer flooding us.
- **logic → writer**: `mpsc::channel(64)`. Bounded. If full, logic emits a `BackpressureDrop` event and refuses to enqueue more low-priority messages.
- **logic → manager (event bus)**: `mpsc::channel(1024)`. Bounded but generous; manager drains continuously.
- **manager → logic (commands)**: `mpsc::channel(32)` per peer.

All channels use Tokio's `mpsc`, never `std::sync::mpsc`. No `parking_lot` locks held across `.await`.

### 3.4 Cancellation

A `CancellationToken` (from `tokio-util`) is shared by all three peer tasks. Any task that decides to disconnect calls `token.cancel()`. The other two observe via `select!` and exit cleanly. The logic task is responsible for flushing the writer's queue (best-effort, with a 1-second deadline) before cancel.

---

## 4. Framing and codec

### 4.1 What `network` already provides

`network::WireMessage::from_bytes(&[u8])` parses a complete header + payload buffer. It does **not** stream. The peer reader cannot use it directly because TCP delivers arbitrary chunk sizes.

### 4.2 `WireCodec` (new, in `peers`)

Implements `tokio_util::codec::Decoder` and `Encoder<Message>`:

```rust
pub struct WireCodec {
    magic: u32,
    state: DecodeState,
}

enum DecodeState {
    AwaitingHeader,
    AwaitingPayload { header: Header },
}
```

`decode(buf: &mut BytesMut) -> Result<Option<WireMessage>>`:

1. In `AwaitingHeader`: if `buf.len() < HEADER_SIZE` return `Ok(None)`. Else parse `Header`, validate magic. If `header.length > MAX_PROTOCOL_MESSAGE_LENGTH (32 MiB)` return `Err(BadHeader)`. Transition to `AwaitingPayload`.
2. In `AwaitingPayload`: if `buf.len() < header.length` return `Ok(None)`. Else extract payload, verify checksum (`payload_checksum(payload) == header.checksum`), build `WireMessage`. Reset to `AwaitingHeader`. Return `Ok(Some(...))`.

`encode(msg: Message, out: &mut BytesMut)` calls `WireMessage::encode(&msg, self.magic)?.to_bytes()` and appends.

`MAX_PROTOCOL_MESSAGE_LENGTH = 32 * 1024 * 1024` matches Bitcoin Core. A block payload near this size is rejected at the codec layer before deserialization.

### 4.3 Framed transport

```rust
let framed = Framed::new(tcp_stream, WireCodec::new(magic));
let (sink, stream) = framed.split();
```

`sink` is owned by the writer task; `stream` by the reader.

### 4.4 Decoding to typed `Message`

Two-step: `WireCodec` produces `WireMessage` (header validated, payload bytes attached). The logic task calls `WireMessage::decode()` which invokes `Message::deserialize_payload(command, &payload)`. This split exists so the reader task does not pay the deserialization cost; the logic task does, on its own time, and can apply banscore on parse failure.

---

## 5. Per-peer state machine

```text
       new socket
           │
           ▼
     ┌──────────┐
     │ Connecting│   (only for outbound; inbound starts at HandshakeAwait)
     └─────┬────┘
           │ TCP up
           ▼
     ┌──────────────┐  send version              ┌──────────────────┐
     │ HandshakeSent├───────────────────────────►│ HandshakeAwaitAck│
     └──────┬───────┘                            └──────────┬───────┘
            │ peer's version arrives                        │ peer's verack arrives
            │ + verack sent                                 ▼
            └─────────────────────────────────────►   ┌────────┐
                                                     │ Ready  │
                                                     └───┬────┘
                                                         │
                              any reason ────────────────▼
                                                ┌──────────────┐
                                                │Disconnecting │
                                                └──────────────┘
```

### 5.1 States in detail

- **Connecting** (outbound only) — TCP dial in flight. Bounded by `OUTBOUND_CONNECT_TIMEOUT = 10s`. On success transition to `HandshakeSent` and send our `version`. On fail emit `Disconnected(DialFailed)`.
- **HandshakeSent** — local `version` is on the wire. Waiting for the peer's `version`. Bounded by `HANDSHAKE_TIMEOUT = 30s`. On peer's `version`: validate (see §5.3), reply with `verack`, transition to `HandshakeAwaitAck`. On any other message: banscore +20 and disconnect.
- **HandshakeAwaitAck** — waiting for peer's `verack`. Same 30s overall timeout. On `verack`: transition to `Ready`, emit `PeerEvent::Ready { peer_id, version_info }`.
- **Ready** — full duplex. Handles all message types. Periodic ping. Idle disconnect after 90 min. Banscore tracked.
- **Disconnecting** — cancel token fired. Writer drains for up to 1s, then sockets close.

Inbound peers skip `Connecting` and start at `HandshakeAwait` (waiting for the remote's `version` first). The local side does not send its `version` until it has received the remote's, mirroring Bitcoin Core.

### 5.2 Handshake message: building our `version`

```rust
VersionMessage {
    version:    PROTOCOL_VERSION,            // 70015
    services:   SERVICES,                    // NODE_NETWORK = 1 only for v0.1
    timestamp:  now_unix(),
    to:         peer_address_as_seen,
    from:       Address::default(),          // not meaningful in modern Bitcoin
    nonce:      random_u64(),                // for self-connection detection
    user_agent: "/Equity:0.1.0/".to_string(),
    height:     local_tip_height,
    relay:      true,
}
```

The nonce is registered in the manager. Receiving a `version` whose nonce matches one we sent within the last 60 seconds indicates a self-connection; disconnect immediately, banscore unchanged.

### 5.3 Handshake validation

- `peer.version >= MIN_PEER_PROTO_VERSION` (default 31800; matches Bitcoin Core's `MIN_PEER_PROTO_VERSION`). Below → disconnect, no ban.
- `peer.timestamp` within ±90 minutes of system clock. If many peers disagree by similar offsets, surface a clock-skew warning. (Bitcoin Core's `nTimeOffset` consensus mechanism — out of scope for v0.1, but log it.)
- `peer.services` recorded; relevant for inventory filtering.
- Reject any peer whose `nonce` matches our outstanding outbound nonces (self-connect).

### 5.4 Per-peer timers (Ready state)

- **Ping ticker** — every 2 min. Generate random nonce, send `ping`, record send-time and nonce.
- **Pong timeout** — if no matching `pong` within 20 min, disconnect with reason `PingTimeout`.
- **Idle timeout** — if no message received in 90 min, disconnect with reason `Idle`.
- **Handshake timeout** — covered above; 30s while in either handshake state.

Implemented via `tokio::time::interval` and `tokio::time::sleep` inside the logic task's `select!`.

---

## 6. Message dispatch

The logic task runs:

```rust
loop {
    tokio::select! {
        msg = inbox.recv()        => handle_incoming(msg)?,
        cmd = cmd_rx.recv()       => handle_command(cmd)?,
        _   = ping_tick.tick()    => send_ping(),
        _   = idle_deadline()     => return disconnect(Idle),
        _   = cancel.cancelled()  => return clean_shutdown(),
    }
}
```

`handle_incoming` per command:

| Command | Action |
| --- | --- |
| `version` | Only valid in `HandshakeSent` / `HandshakeAwait`. Elsewhere: banscore +1, ignore. |
| `verack` | Only valid in `HandshakeAwaitAck`. Elsewhere: banscore +1, ignore. |
| `ping` | Reply with matching `pong`. |
| `pong` | Match against outstanding pings; remove from set. Unknown nonce: banscore +1. |
| `addr` | Forward to `addrman` as `AddrLearned { source: peer_id, addrs }`. Cap at 1000 entries per message. |
| `getaddr` | Reply with up to 1000 addresses from `addrman`. Rate-limited: at most once per 24 h per peer. |
| `inv`, `getdata`, `notfound` | Forward to `sync` as-is. Cap entries at 50_000. |
| `getblocks`, `getheaders` | Forward to `sync`. |
| `headers`, `block`, `tx`, `merkleblock` | Forward to `sync`. |
| `reject` | Log at `debug`. Not forwarded. |
| `filterload`, `filteradd`, `filterclear` | Per-peer Bloom filter state (BIP-37). Out of scope v0.1 — log and ignore. |
| `sendheaders` | Set per-peer flag; future `inv`-of-block becomes `headers` push. v0.1: record but no behavior change yet. |
| `mempool` | Forward to `mempool`. |
| `alert` | Deprecated; log at `debug`. |
| unknown | banscore +1 in v0.1 (Bitcoin Core ignores; we are stricter while debugging). |

### 6.1 Decoding failures

A `WireCodec` decode error (bad magic, oversize, bad checksum) is treated as protocol violation: emit `Disconnected(BadFrame)` and let the manager apply banscore (+100, instant ban). A `Message::deserialize_payload` failure (command known but payload malformed) is also instant-ban (+100). An unknown command is +1 only.

### 6.2 Send queue

`PeerCommand::Send(Message, Priority)` arrives from the manager. `Priority` is one of:

- `Control` — handshake, ping/pong. Never dropped.
- `Critical` — block, headers in response to explicit request. Never dropped.
- `Normal` — inv broadcasts, tx relay. Dropped if writer queue is full.

The logic task picks priority before pushing to writer.

---

## 7. Peer manager

Top-level type owned by the node binary:

```rust
pub struct PeerManager {
    inbound:   TcpListener,
    peers:     RwLock<HashMap<PeerId, PeerHandle>>,
    outbound_target: usize,        // default 8
    inbound_max:     usize,        // default 125 - outbound_target
    addrman:   Arc<AddrMan>,
    banlist:   Arc<BanList>,
    events_tx: mpsc::Sender<PeerEvent>,
    nonces:    Mutex<HashMap<u64, Instant>>,   // self-connect detection
    config:    NetworkConfig,
}
```

### 7.1 Listener loop

```rust
loop {
    let (sock, addr) = inbound.accept().await?;
    if banlist.is_banned(&addr.ip()) { continue; }
    if rate_limiter.check(&addr.ip()).is_err() { continue; }
    if self.peers.read().len() >= inbound_max + outbound_target { continue; }
    self.spawn_peer(sock, Direction::Inbound, addr);
}
```

Per-source-IP rate limiter: token bucket, 4 connects/min/IP. Excess silently dropped — no banscore (could be NAT'd users).

### 7.2 Outbound dialer

A separate task. Wakes every 2s or on `PeerEvent::Disconnected`:

```rust
while self.outbound_count() < self.outbound_target {
    let Some(addr) = self.addrman.select_outbound() else { break };
    if self.banlist.is_banned(&addr.ip()) { continue; }
    self.spawn_peer_dial(addr);
}
```

`AddrMan::select_outbound` weighs by recency and success rate; see §9.

### 7.3 Spawning a peer

```rust
fn spawn_peer(&self, sock: TcpStream, dir: Direction, addr: SocketAddr) {
    let id = PeerId::next();
    let token = CancellationToken::new();
    let (cmd_tx, cmd_rx) = mpsc::channel(32);
    let handle = PeerHandle { id, addr, dir, cmd_tx, token: token.clone(), ... };
    self.peers.write().insert(id, handle);

    let codec = WireCodec::new(self.config.magic);
    let framed = Framed::new(sock, codec);
    let (sink, stream) = framed.split();

    tokio::spawn(reader_task(stream, inbox_tx, token.clone()));
    tokio::spawn(writer_task(sink, outbox_rx, token.clone()));
    tokio::spawn(logic_task(state, inbox_rx, cmd_rx, outbox_tx, self.events_tx.clone(), token));
}
```

`PeerId` is a `u64` monotonic counter; cheap, never reused per process.

### 7.4 Event bus

`PeerEvent` is the public boundary:

```rust
pub enum PeerEvent {
    Connected     { id: PeerId, addr: SocketAddr, dir: Direction },
    Ready         { id: PeerId, info: VersionInfo },
    Message       { id: PeerId, msg: Message },
    Banscore      { id: PeerId, delta: u32, reason: &'static str },
    Disconnected  { id: PeerId, reason: DisconnectReason },
}
```

`sync` subscribes to this stream. The manager aggregates banscore per peer and triggers disconnect at threshold; this happens internally, not in `sync`.

### 7.5 Disconnect reasons

```rust
pub enum DisconnectReason {
    PeerRequested,    // peer closed cleanly
    Idle,             // 90 min no traffic
    PingTimeout,      // 20 min no pong
    HandshakeTimeout, // 30 min in handshake
    BadFrame,         // codec error
    BadMessage,       // deserialization error
    Banned,           // banscore >= 100
    DialFailed,       // outbound connect failed
    SelfConnect,
    Eviction,         // we needed the slot
    LocalShutdown,
}
```

---

## 8. Banscore and banlist

### 8.1 Banscore deltas

Per peer, a `u32` counter starting at 0. Threshold 100 = ban. Sources:

| Event | Delta |
| --- | --- |
| Bad frame (codec) | +100 |
| Bad message (parse) | +100 |
| Bad PoW / merkle on supplied block (from validator) | +100 |
| Bad signature on supplied tx | +100 |
| Unknown command | +1 |
| Wrong-state message (e.g. `verack` before `version`) | +1 |
| `pong` with unknown nonce | +1 |
| Exceeds per-message item cap (e.g. 50k inv) | +50 |

Validator-level events arrive as `PeerCommand::Banscore` from `sync`.

### 8.2 Ban store

`banlist.json` in data dir. Entries: `{ ip: IpAddr, until: SystemTime, reason: String }`. Default ban duration 24 h. Loaded at startup, written on each new ban (debounced 5 s). Cleared lazily — expired entries removed when checked.

Subnet-level bans (`/16` for IPv4, `/32` for IPv6) are configurable but disabled by default.

### 8.3 Eviction

When inbound is full and a new well-reputed peer wants in, we evict an existing peer. Bitcoin Core's algorithm (preserve the 4 youngest, 4 with most blocks, 8 best ping, etc.) is the eventual target. v0.1 uses a simpler heuristic: evict the longest-idle inbound peer that hasn't sent a useful message (block, headers, tx) in the last 30 min.

---

## 9. Address manager (`addrman`)

### 9.1 Goal

Maintain a set of peer addresses across restarts. Bias outbound selection toward addresses likely to be alive and not under attacker control.

### 9.2 Data structure

Two tables, adapted from Bitcoin Core's `CAddrMan`:

- **New** — heard about but not yet successfully connected to. Bucketed by source-IP group (`/16` for IPv4, `/32` for IPv6). 1024 buckets × 64 slots.
- **Tried** — successfully connected to at least once. Bucketed by the address's own IP group. 256 buckets × 64 slots.

Each slot holds at most one entry. Collisions trigger pseudo-random replacement weighted by recency of last success.

### 9.3 Operations

```rust
impl AddrMan {
    pub fn add(&self, addrs: &[Address], source: IpAddr);
    pub fn mark_attempted(&self, addr: SocketAddr, when: SystemTime);
    pub fn mark_good(&self, addr: SocketAddr, when: SystemTime);   // promotes to Tried
    pub fn select_outbound(&self) -> Option<SocketAddr>;           // weighted random
    pub fn get_addr_response(&self, max: usize) -> Vec<Address>;   // for getaddr replies
    pub fn save(&self) -> Result<()>;                              // dump peers.dat
    pub fn load(path: &Path) -> Result<Self>;
}
```

### 9.4 Persistence

`peers.dat`. Custom format, not Bitcoin Core compatible. Layout:

```text
magic: u32 LE (0x45515459 = "EQTY")
version: u8 (1)
flags: u8 (reserved)
n_new: u32 LE
n_tried: u32 LE
[entries: Address + last_success: u64 + last_attempt: u64 + n_attempts: u32]
checksum: 4 bytes SHA-256d of preceding bytes
```

Written on graceful shutdown and every 15 min as a safety net. Read once at startup; on checksum failure, log warning and start fresh.

### 9.5 Seeding

If `peers.dat` is empty (fresh install) and no DNS seeds configured, the node has no way to find peers. Options:

- **DNS seeds** — fixed list of hostnames that resolve to seed-node IPs. Bitcoin Core's set (`seed.bitcoin.sipa.be`, etc.) is the obvious starting set.
- **Hard-coded fallback** — last-resort embedded list of a few well-known IPs.
- **CLI override** — `--connect=<host:port>` forces outbound to a specific peer, bypasses addrman entirely.

v0.1: implement CLI override and hard-coded fallback. DNS seeds deferred — see §15.4.

---

## 10. Inbound rate limiting

### 10.1 Per-source connect rate

Token bucket per `IpAddr` (or `/16` for IPv4). 4 tokens, refill 4 per minute. Connect attempts that fail the bucket are dropped silently before the handshake. Prevents trivial connection-flood DoS without falsely banning a NAT.

### 10.2 Per-peer message rate

Tokio's `Framed` stream is read greedily, but the reader→logic channel has capacity 128. If the logic task is slow (e.g. validating a block), the reader awaits — the TCP receive buffer fills, then kernel applies backpressure to the sender. Peers that genuinely flood eventually trip the 32 MiB protocol message cap (codec error → ban).

### 10.3 Resource caps

- Per-peer inbox: 128 messages.
- Per-peer outbox: 64 messages.
- Per-peer outstanding pings: 8.
- Per-peer outstanding inv requests (driven by `sync`, not the peer layer, but enforced here): 1024.

---

## 11. Wire protocol details

### 11.1 Network magic

- Mainnet `0xD9B4BEF9`, port 8333.
- Testnet3 `0x0709110B`, port 18333.
- Regtest `0xDAB5BFFA`, port 18444.

Magic value selected at node startup based on `--network`. The codec rejects frames with mismatched magic — this gives early failure when a node is accidentally connected to the wrong network.

### 11.2 Protocol version

`PROTOCOL_VERSION = 70015` for v0.1. Sufficient for `sendheaders`, `mempool`, and full BIP-31 (`ping`/`pong` with nonce). BIP-37 Bloom filter messages are parsed and ignored; we do not advertise `NODE_BLOOM`.

### 11.3 Service flags advertised

`NODE_NETWORK (1)` only. Specifically **not** advertised:

- `NODE_BLOOM (4)` — Bloom filtering disabled.
- `NODE_WITNESS (8)` — SegWit not implemented in v0.1.
- `NODE_NETWORK_LIMITED (1024)` — we are archival (until pruning lands).

### 11.4 Caps adopted from Bitcoin Core

| Cap | Value |
| --- | --- |
| `MAX_PROTOCOL_MESSAGE_LENGTH` | 32 MiB |
| `MAX_INV_SZ` | 50_000 |
| `MAX_ADDR_TO_SEND` | 1_000 |
| `MAX_HEADERS_RESULTS` | 2_000 |
| `MAX_LOCATOR_SZ` | 101 |
| `MAX_BLOCK_SERIALIZED_SIZE` | 1_000_000 (pre-SegWit) |

Frames or messages that exceed their cap: banscore +50 plus disconnect.

---

## 12. Threading and shared state

### 12.1 Ownership

- `PeerManager` is owned by the node binary inside `Arc`.
- `AddrMan` likewise `Arc<AddrMan>` with internal `parking_lot::Mutex`. Operations are short; no awaits while holding the lock.
- `BanList` likewise.
- Per-peer state is **not** behind a lock — only the logic task touches it.

### 12.2 Lock discipline

- No `parking_lot` lock held across `.await`.
- No nesting of `AddrMan` and `BanList` locks.
- `PeerManager.peers` (a `RwLock<HashMap>`) is read on the hot path (dispatch a command to a peer's `cmd_tx`); write only on connect/disconnect.

### 12.3 Send and Sync

All public types (`PeerManager`, `AddrMan`, `BanList`, `PeerHandle`) are `Send + Sync`. `PeerCommand` and `PeerEvent` are `Send` and ideally `Sync` to allow channel fan-out.

---

## 13. Error model

### 13.1 Crate error

```rust
#[derive(Debug, thiserror::Error)]
pub enum NetError {
    #[error("io: {0}")]              Io(#[from] std::io::Error),
    #[error("codec: {0}")]           Codec(CodecError),
    #[error("handshake timed out")]  HandshakeTimeout,
    #[error("self-connection")]      SelfConnect,
    #[error("banned")]               Banned,
    #[error("peer not found")]       NoSuchPeer,
    #[error("channel closed")]       ChannelClosed,
}
```

Public API at the manager level uses `anyhow::Result` per workspace policy, with `NetError` as the underlying type when discrimination matters internally.

### 13.2 What is never a `Result`

- `PeerEvent::Banscore` is an event, not an error. Validators returning `ConsensusError` are translated to events in `sync` before reaching this layer.
- Channel-full on the outbox is a drop (with logged event), not an error.

### 13.3 Panics

Same rule as the rest of the workspace: panics indicate bugs, not bad input. Network code never panics on attacker-controlled data.

---

## 14. Configuration

Subset of `NetworkConfig` consumed by this layer:

| Key | Default | Notes |
| --- | --- | --- |
| `magic` | mainnet | derived from `--network` |
| `port` | 8333 | listener port |
| `bind` | `0.0.0.0` | listener bind |
| `outbound_target` | 8 | desired outbound peer count |
| `max_connections` | 125 | inbound + outbound cap |
| `connect` | `[]` | explicit peer list; if set, addrman is bypassed and only these are dialed |
| `add_node` | `[]` | additional peers always retried |
| `seed_nodes` | hard-coded | fallback if addrman is empty |
| `dns_seeds` | `[]` | disabled by default in v0.1 |
| `user_agent` | `/Equity:0.1.0/` | sent in `version` |
| `ban_duration_secs` | 86_400 | default ban length |
| `handshake_timeout_secs` | 30 | |
| `ping_interval_secs` | 120 | |
| `idle_timeout_secs` | 5_400 | 90 min |

---

## 15. Open questions

1. **Self-connection nonce TTL.** Bitcoin Core uses 60s; verify on slow networks.
2. **Eviction algorithm.** Implement Bitcoin Core's full eviction or stick with simple-idle for v0.1?
3. **Tor / I2P support.** Out of scope v0.1 but the `Address` type uses 16-byte IPv6, leaving no room for onion v3. Future onion support implies a wider address type and a separate transport plug.
4. **DNS seeds.** Are we comfortable hard-coding Bitcoin Core's seed list? Alternative is requiring `--connect` on first run.
5. **Compact block relay (BIP-152).** When? v0.2?
6. **Block-relay-only peers.** Bitcoin Core dedicates 2 outbound slots to block-only peers for anti-eclipse. Worth adopting v0.1.
7. **AddrV2 (BIP-155).** New address message format for Tor/I2P. Adoption tied to (3).
8. **Encrypted transport (BIP-324, v2 P2P).** Strong upgrade path; design space we should keep open.
9. **Anchor connections.** Bitcoin Core persists 2 outbound peers across restarts to prevent eclipse. Easy to add once addrman is solid.
10. **Per-peer thread cost.** Three tasks per peer × 125 peers = 375 tasks. Cheap with Tokio, but at the 1000-peer scale (test scenarios) it may matter. Profile before optimizing.

---

## 16. How this satisfies the technical design

| Technical-design §  | Covered here in § |
| --- | --- |
| §4.8.1 per-peer state machine | §5 |
| §4.8.2 async architecture | §3 |
| §4.8.3 peer manager | §7 |
| §4.8.4 banscore | §8 |
| §4.9 address manager | §9 |
| §5.1 single Tokio runtime | §3.1 |
| §5.2 shared state, no locks across `.await` | §12.2 |
| §5.3 backpressure | §10 |
| §6 error model | §13 |
| §8 security: untrusted input, DoS surfaces | §4.2, §10, §11.4 |

---

## 17. Phase order

Implement bottom-up to keep each phase testable in isolation.

1. **`WireCodec`** — `tokio_util::codec` impls, unit-tested against pre-built frames. No I/O yet.
2. **`peers::Peer`** state machine in a single task — drive against `tokio::io::duplex()` pipes. No real sockets. Two in-process Peers complete a handshake.
3. **`PeerManager` listener + dialer** — real TCP, but loopback only. Connect to a regtest-mode `bitcoind` for ground truth.
4. **`BanList`** — JSON persistence, banscore plumbing.
5. **`AddrMan`** — in-memory tables + `peers.dat` persistence. Seed via CLI `--connect`.
6. **Inbound rate limiter** — token bucket per IP.
7. **DNS seeds + hard-coded fallback** — gate-controlled by open question (4).

Each phase ships with proptest and integration tests under `libs/peers/tests/`.

---

## 18. References

- Bitcoin Core P2P implementation: `src/net.cpp`, `src/net_processing.cpp`, `src/addrman.cpp`.
- btcd: `peer/` package, especially `peer.go` (per-peer FSM) and `connmgr/` (connection manager).
- BIP-31 (`pong` with nonce), BIP-37 (Bloom filter), BIP-130 (`sendheaders`), BIP-152 (compact blocks), BIP-155 (AddrV2), BIP-324 (encrypted v2 transport).
- libbitcoin's `network/` module — clean separation of session/protocol/channel layers.

---

## 19. Change log

| Date | Change | Author |
| --- | --- | --- |
| 2026-05-26 | Initial draft. | Claude (Opus 4.7), session-authored |
