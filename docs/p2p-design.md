# Equity — P2P Network Layer Design

Version: 0.2 (draft)
Status: design, not implemented.
Companion to [technical-design.md](technical-design.md) (§3.3 library/application boundary, §4.8 peer state machine, §4.14 transport interface, §4.9 addrman, §4.10 sync) and [blockchain-plan.md](blockchain-plan.md).

This document specifies the **networking layer that lives in `apps/node`** — how Equity opens TCP connections to Bitcoin peers, frames bytes into messages, drives the handshake, maintains a population of peers, and defends against misbehavior. It is **the application side of the library/application boundary**: every type and task described here lives in `apps/node`, not in any library crate.

The library side of the boundary — the `Transport` trait, `NetEvent` enum, and `PeerId` type — is specified in [technical-design.md §4.14](technical-design.md). Read that section first; this document explains how the application implements those types.

It does **not** specify chain validation, block download scheduling, or mempool logic — those live in `sync` / `validator` / `mempool`. The network layer is intentionally dumb about chain semantics; it speaks bytes, not consensus.

## 0. Library / Application split (binding constraint)

By project rule (technical-design §3.3), **no library crate touches a socket.** This document therefore describes the application-side implementation of the network. The library expresses its needs through the `p2p::Transport` trait (commands) and a `mpsc::Receiver<NetEvent>` (events); the application — everything in this document — implements the trait and produces the events.

Three concrete consequences flow from this rule and shape the rest of the design:

1. `TokioTransport` (§7), `WireCodec` (§4), the listener/dialer (§7), the per-peer task trio (§3), the `BanList` (§8), and the rate limiters (§10) all live in `apps/node`.
2. The `Peer` state machine (`libs/peers`, technical-design §4.8) is **pure logic** — it accepts `Message` values and returns `PeerAction` values. The reader/writer/logic task trio described in this document is the app-side wiring that connects that pure logic to real sockets.
3. `AddrMan` (§9) is a pure data structure in `libs/addrman` (no I/O), but its persistence (`peers.dat` reads/writes) is performed by `apps/node` calling sync save/load methods on the library type.

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
- Fee filtering (BIP-133 `feefilter`), AddrV2 (BIP-155), wtxidrelay (BIP-339) — follow-ups (see §11.5).
- Bloom filter advertisement (BIP-37 `NODE_BLOOM`) — out of scope; messages parsed but no filter state maintained.

### 1.3 Module placement

The networking subsystem is split between library crates (pure logic, no I/O) and the application (sockets, async runtime, transport implementation):

#### Library crates

| Crate | Role |
| --- | --- |
| `network` (exists, library) | Pure data types and serializers — `Header`, `WireMessage`, `Message`, `Address`, `InventoryId`. **No I/O.** |
| `p2p` (exists, library) | Wire-format primitives **plus** the `transport` module that defines `Transport`, `NetEvent`, `PeerId`, `Direction`, `VersionInfo`, `DisconnectReason`, `TransportError`. Full spec in [technical-design.md §4.14](technical-design.md). **No I/O.** |
| `peers` (new, library) | Pure-logic per-peer state machine. Takes decoded `Message` values, returns `PeerAction` values (Send/Disconnect/Ban/Emit). **No tasks, no sockets.** |
| `addrman` (new, library) | Address book as a `Send + Sync` data structure. Persistence routines (`save`, `load`) are synchronous; the application chooses when to call them. **No I/O scheduling.** |

#### Application (`apps/node`)

| Module | Role |
| --- | --- |
| `apps/node/src/transport.rs` | `TokioTransport` — implements `p2p::Transport`. Owns per-peer `mpsc::Sender<Message>` handles. |
| `apps/node/src/codec.rs` | `WireCodec` — `tokio_util::codec::Decoder` / `Encoder<Message>` over the existing `network::WireMessage` serializers. |
| `apps/node/src/peer.rs` | Per-peer task trio (reader, writer, logic). Wraps a `peers::Peer` (the library FSM) and runs it against a real `TcpStream`. |
| `apps/node/src/manager.rs` | Peer manager — listener, dialer, registry, eviction. |
| `apps/node/src/banlist.rs` | Persistent banlist (`banlist.json`). |
| `apps/node/src/rate_limit.rs` | Per-IP token bucket. |

`sync` (a library crate) consumes `peers` and `addrman` directly, and talks to the network via the `p2p::Transport` trait. The application provides the `TokioTransport` instance to `sync` at startup. `peers` never calls into `sync` directly; sync subscribes to a `NetEvent` stream.

---

## 2. Layered view

```text
+================ libs/sync (library) ================+
|  consumes NetEvent stream                           |
|  issues commands via p2p::Transport trait           |
+=====================================================+
                ▲                  │
                │ NetEvent         │ Transport::{send,ban,disconnect}
                │                  ▼
+================ apps/node (application) ==================+
|  TokioTransport       (impl p2p::Transport)              |
|  PeerManager          (listener, dialer, registry)       |
|  per-peer task trio   (reader, writer, logic)            |
|     each logic task hosts a libs/peers::Peer FSM         |
|  WireCodec            (length-prefixed Header + payload) |
|  BanList, rate limiter                                   |
|  tokio::net::TcpStream / TcpListener                     |
+==========================================================+
```

The doubled box top divides the library (above) from the application (below). The two arrows crossing the divide are the **only** interactions allowed between the layers.

Data flow:

- **Inbound**: bytes → `WireCodec::decode` (app) → `WireMessage` (app) → `Message::deserialize_payload` (lib serializer) → forwarded to `peers::Peer::handle` for state-machine bookkeeping → emitted as `NetEvent::Message` on the events channel → consumed by `sync`.
- **Outbound**: `sync` calls `transport.send(peer, msg)` (lib → app) → `TokioTransport::send` enqueues into the per-peer `mpsc::Sender<Message>` → per-peer writer task encodes via `WireCodec` → bytes on socket.

Errors at the codec or message layer never reach `sync` as a panic; they become a `NetEvent::Disconnected { reason: BadFrame | BadMessage | … }`. The application alone is responsible for the per-IP `BanList`; the library can request a ban via `Transport::ban` but never persists one itself.

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

- **Ping ticker** — every 2 min (matches Bitcoin Core's `PING_INTERVAL`). Generate random nonce, send `ping`, record send-time and nonce.
- **Pong timeout** — if no matching `pong` within 20 min, disconnect with reason `PingTimeout`.
- **Idle timeout** — if no message received in 90 min, disconnect with reason `Idle`. Matches the devguide's "If 90 minutes pass without a message being received by a peer, the client will assume that connection has closed."
- **Keep-alive convention** — the devguide also notes "Nodes by default will send a message to peers before 30 minutes of inactivity"; our 2-minute ping easily satisfies that.
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
| unknown | Silently ignored in v0.1. See §11.6 for why we relax the strict-debug rule. |

### 6.1 Decoding failures

A `WireCodec` decode error (bad magic, oversize, bad checksum) is treated as protocol violation: emit `Disconnected(BadFrame)` and let the manager apply banscore (+100, instant ban). A `Message::deserialize_payload` failure (command known but payload malformed) is also instant-ban (+100). An unknown command is +1 only.

### 6.2 Orphan-block handling

The devguide distinguishes orphan blocks (no known parent header) from stale blocks (known parent, not on best chain). Equity follows the headers-first rule: **discard any unsolicited `block` message whose previous-block hash is not already in our header index.** This is enforced in `sync`, not here, but the peer layer forwards the block as-is — it does not pre-filter, because the header-index lookup is the sync layer's responsibility.

Consequence: an unsolicited block push from a miner (the "spontaneous `block` message" path described in the devguide) is dropped silently if we have not first received the corresponding header through `headers` or `inv` → `getheaders` → `headers`. The peer is **not** banned for this; spontaneous pushes are legal in the protocol, just unhelpful for a headers-first node.

### 6.3 Send queue

`PeerCommand::Send(Message, Priority)` arrives from the manager. `Priority` is one of:

- `Control` — handshake, ping/pong. Never dropped.
- `Critical` — block, headers in response to explicit request. Never dropped.
- `Normal` — inv broadcasts, tx relay. Dropped if writer queue is full.

The logic task picks priority before pushing to writer.

---

## 7. Peer manager (`apps/node/src/manager.rs`)

The peer manager is **part of the application, not a library crate.** It owns sockets, spawns tasks, and provides the listener/dialer behavior. It is also the value that implements `p2p::Transport` (via composition with `TokioTransport`).

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

If `peers.dat` is empty (fresh install) and no DNS seeds configured, the node has no way to find peers. Options and the order Bitcoin Core uses, per the devguide:

1. **Try the on-disk address database** for up to 11 seconds. If we get connections, proceed without seeds.
2. **Query DNS seeds** — fixed list of hostnames that resolve to seed-node IPs (Bitcoin Core's set: `seed.bitcoin.sipa.be`, `dnsseed.bluematt.me`, `dnsseed.bitcoin.dashjr.org`, etc.). 60-second timeout before giving up.
3. **Fall back to a hard-coded peer list** — last-resort embedded list of a few well-known IPs that were active around release time.

The devguide is explicit: "DNS seed results are not authenticated and a malicious seed operator or network man-in-the-middle attacker can return only IP addresses of nodes controlled by the attacker." Treat first-contact peers as untrusted and weight outbound selection toward the tried table once we have one.

CLI override (`--connect=<host:port>`) bypasses all three steps and dials exactly the listed peers — useful for development and testing.

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

| Cap | Value | Source |
| --- | --- | --- |
| `MAX_PROTOCOL_MESSAGE_LENGTH` | 32 MiB | Bitcoin Core |
| `MAX_INV_SZ` | 50_000 | Bitcoin Core |
| `MAX_ADDR_TO_SEND` | 1_000 | Bitcoin Core |
| `MAX_HEADERS_RESULTS` | 2_000 | devguide §IBD headers-first ("maximum response") |
| `MAX_GETBLOCKS_RESULTS` | 500 | devguide §IBD blocks-first ("maximum response to a getblocks message") |
| `MAX_LOCATOR_SZ` | 101 | Bitcoin Core |
| `MAX_BLOCK_SERIALIZED_SIZE` | 1_000_000 (pre-SegWit) | consensus |
| `MAX_INFLIGHT_BLOCKS_PER_PEER` | 16 | devguide ("up to 16 blocks at a time from a single peer") |
| `IBD_DOWNLOAD_WINDOW` | 1_024 | devguide ("1,024-block moving download window") |
| `BLOCK_STALLING_TIMEOUT` | 2 s extra wait then disconnect | devguide ("a minimum of two more seconds for the stalling node") |

Frames or messages that exceed their cap: banscore +50 plus disconnect.

`MAX_INFLIGHT_BLOCKS_PER_PEER`, `IBD_DOWNLOAD_WINDOW`, and `BLOCK_STALLING_TIMEOUT` are enforced in `sync`, not here, but listed for completeness — the peer layer must not refuse legitimate `getdata` traffic that respects these caps.

### 11.5 Messages not implemented in v0.1

The current `network::messages::Message` enum (22 variants) covers the classic message set up to roughly Bitcoin Core 0.9. The following modern messages are **deferred**; the codec must currently treat them as "unknown command" (+1 banscore — see §11.6 below for why this is loosened in practice).

| Message | BIP | Purpose | Plan |
| --- | --- | --- | --- |
| `feefilter` | BIP-133 | Advertise minimum relay fee | v0.2; cheap addition |
| `sendcmpct`, `cmpctblock`, `getblocktxn`, `blocktxn` | BIP-152 | Compact block relay | v0.2 |
| `sendaddrv2`, `addrv2` | BIP-155 | Tor v3 + I2P + larger addresses | gated on Tor/I2P support |
| `wtxidrelay` | BIP-339 | wtxid-based inv (SegWit-aware) | gated on SegWit |
| `getcfilters`, `cfilter`, `getcfheaders`, `cfheaders`, `getcfcheckpt`, `cfcheckpt` | BIP-157/158 | Compact block filters | not planned for v0.1 |

### 11.6 Unknown-command policy reconciliation

§6 defines an unknown command as banscore +1. Modern Bitcoin Core sends `feefilter` and `sendcmpct` early in the handshake; a strict v0.1 implementation would accumulate banscore on every connection. Resolution: **unknown commands count as +0 in v0.1.** The +1 rule is reinstated when the message set is brought up to date (after BIP-133 and BIP-152 land). This is the one practical relaxation to the design and matches Bitcoin Core's `ProcessMessage` behavior, which silently ignores unknown commands.

---

## 12. Threading and shared state

### 12.1 Ownership

- `PeerManager` lives in **`apps/node`** and is owned by the node binary inside `Arc`. It is not a library type.
- `TokioTransport` lives in **`apps/node`**, owned by the node binary, shared with library consumers as `Arc<dyn p2p::Transport>`.
- `AddrMan` is a **library** type (`libs/addrman`). The node holds `Arc<AddrMan>` with internal `parking_lot::Mutex`. Operations are short and synchronous; the application is responsible for calling `save()` / `load()` at the right times. No awaits while holding the lock.
- `BanList` lives in **`apps/node`**. Library logic requests bans through `Transport::ban`.
- Per-peer state inside a `peers::Peer` (library FSM) is **not** behind a lock — only the per-peer logic task touches it, and that task wraps the library `Peer` value by move, not by reference.

The lib/app rule (§0 of this document, technical-design §3.3) is enforced by the type signatures: the only `Arc<dyn Transport>` outbound from the application is the trait object, and the library cannot downcast it back to `TokioTransport`. No library crate links `tokio` or `tokio-util` directly.

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
| §3.3 library / application boundary | §0, §1.3, §2, §12.1 |
| §4.2 p2p crate (transport module) | §0, referenced throughout |
| §4.8.1 per-peer state machine (pure logic, `libs/peers`) | §5 — wired into the app-side trio described in §3 |
| §4.8.2 async architecture (now app-side) | §3 |
| §4.8.3 peer manager (app-side) | §7 |
| §4.8.4 banscore | §8 |
| §4.9 address manager | §9 |
| §4.13 node binary | §1.3, §7 |
| §4.14 transport interface (`p2p::Transport`, `NetEvent`) | implemented by §7 (`TokioTransport`); produced by per-peer tasks (§3.2) |
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

## 18. Discrepancies with the bitcoin.org devguide

A read of <https://developer.bitcoin.org/devguide/p2p_network.html> against the current code (`libs/network`) plus this design surfaced the following gaps. Each is tagged **CODE** (the existing implementation does not match the devguide) or **DESIGN** (this document had to be updated to match the devguide).

### 18.1 Code-level discrepancies

**CODE 1 — Modern messages missing from `network::messages::Message`.** The enum has 22 variants and stops roughly at Bitcoin Core 0.9. The devguide and current Bitcoin Core both rely on:

- `feefilter` (BIP-133) — sent very early after `verack` by Core ≥ 0.13. Equity will see it on every modern peer.
- `sendcmpct`, `cmpctblock`, `getblocktxn`, `blocktxn` (BIP-152) — sent by Core ≥ 0.13. Without these we cannot opt into compact block relay.
- `sendaddrv2`, `addrv2` (BIP-155) — sent by Core ≥ 22.0. Needed for Tor v3 / I2P addresses.
- `wtxidrelay` (BIP-339) — sent by Core ≥ 0.21 before `verack`. SegWit-aware tx inv.

Impact today: parsing produces "unknown command" for each. With the original +1 banscore rule, a normal mainnet peer would accumulate ≥ 5 banscore inside the first second after handshake. §11.6 relaxes the rule to 0 until the message set is updated.

**CODE 2 — `MerkleBlockMessage` lacks `to_json`.** Every other message payload in `libs/network/src/messages.rs` implements `to_json` for diagnostics; `MerkleBlockMessage` does not. Cosmetic, not protocol-breaking.

**CODE 3 — `AlertMessage` is still in the enum.** The devguide states the alert system was "Removed in Bitcoin Core 0.13.0". Equity parses it for backward compatibility. This is fine, but the design should note `alert` is wire-deprecated and we will never originate it.

**CODE 4 — `VersionMessage` field gating uses height thresholds matching the wire spec but not the consensus.** Code gates `from`/`nonce`/`user_agent`/`height` on `version >= 106` and `relay` on `version >= 70001`. These match the historical Bitcoin protocol versions. No discrepancy; flagged for awareness.

**CODE 5 — `Address` type is fixed at 16-byte IPv6.** This is consistent with the original `addr` message but not with BIP-155 `addrv2`, which carries variable-length addresses (Tor v3 = 32 bytes, I2P = 32 bytes, Cjdns = 16 bytes). Tor support implies a new address type and a parallel `addrv2`/`sendaddrv2` codec path.

**CODE 6 — `network::messages::Message` enum uses string command dispatch.** `Message::deserialize_payload(command: &str, payload: &[u8])`. The devguide command names match what the code expects (lower-case ASCII, null-padded in the header). No discrepancy; flagged because the command-string set must be kept in sync as new BIPs are added.

**CODE 7 — `MAX_PROTOCOL_MESSAGE_LENGTH` cap is not enforced in code.** `WireMessage::from_bytes` only checks `s.len() < header.length`. A malicious peer can announce a 4 GiB payload and force allocation of a corresponding receive buffer before the framing layer detects the size. The new `WireCodec` (§4.2) fixes this; the existing helper should also gain the check.

**CODE 8 — `WireMessage::from_bytes` is whole-buffer, not streaming.** Useable for tests but unusable on a real socket. Acknowledged in §4.1 and addressed by `WireCodec`.

### 18.2 Design-level discrepancies (this document, before today's update)

**DESIGN 1 — Caps table was incomplete.** Added: `MAX_GETBLOCKS_RESULTS = 500`, `MAX_INFLIGHT_BLOCKS_PER_PEER = 16`, `IBD_DOWNLOAD_WINDOW = 1024`, `BLOCK_STALLING_TIMEOUT = 2 s`. The 1024-block moving window and the 16-per-peer in-flight cap together give the devguide's 128 simultaneous blocks (16 × 8 outbound).

**DESIGN 2 — Keep-alive wording.** Original draft did not link our 2-min ping to the devguide's "30 minutes of inactivity" rule. Now noted in §5.4.

**DESIGN 3 — Orphan-block discard.** Original did not state the headers-first orphan-discard rule. Now §6.2.

**DESIGN 4 — Seeding order.** Original listed DNS/hard-coded/CLI but did not record the devguide's "11 s peer-db, then 60 s DNS, then hard-coded" sequence. Now §9.5.

**DESIGN 5 — Unknown-command policy contradiction.** §6 said +1; modern peers send 4–5 unknown commands during handshake (CODE 1). Reconciled in §11.6 by moving to "ignore" until the message set is current.

**DESIGN 6 — IBD trigger thresholds.** The devguide gives explicit triggers: tip's header time more than 24 h in the past, or local block chain more than 144 blocks below local header chain. These live in `sync`, not the peer layer, but are recorded here so the sync designer does not have to re-derive them.

### 18.3 Discrepancies the devguide does not address

The devguide is silent on several modern protocol details Equity will need. Listed here so they are not mistaken for design omissions:

- Network magic byte values (Equity uses the canonical values; see §11.1).
- Service flag bit numbers (`NODE_NETWORK = 1`, `NODE_BLOOM = 4`, `NODE_WITNESS = 8`, `NODE_NETWORK_LIMITED = 1024`).
- IPv4-mapped-IPv6 encoding in `addr` payload (`::ffff:a.b.c.d`).
- BIP-31 nonce-bearing `ping`/`pong` (devguide does not mention).
- BIP-324 v2 encrypted transport.
- BIP-152 high-bandwidth vs low-bandwidth relay modes.

These are pulled from the BIP texts, Bitcoin Core source, and BIP-150/151 (retired) for context.

---

## 19. References

- [`github.com/bitcoin/bips`](https://github.com/bitcoin/bips). **Authoritative source for every BIP referenced below.** Always read the BIP file directly before implementing the message it specifies. Example path: `bips/bip-0152.mediawiki` for compact block relay.
- [Bitcoin developer guide — P2P network](https://developer.bitcoin.org/devguide/p2p_network.html). Primary narrative reference for handshake, IBD, block and tx relay, banning. Cited inline throughout §5, §6, §9, §11.4.
- [Bitcoin developer reference — P2P messages](https://developer.bitcoin.org/reference/p2p_networking.html). Wire-format details for every message; the authoritative source when the devguide is too high-level.
- Bitcoin Core P2P implementation: `src/net.cpp`, `src/net_processing.cpp`, `src/addrman.cpp`.
- btcd: `peer/` package, especially `peer.go` (per-peer FSM) and `connmgr/` (connection manager).
- BIPs cited in this document, in numerical order (each link points to the file in `github.com/bitcoin/bips`):
  - [BIP-31](https://github.com/bitcoin/bips/blob/master/bip-0031.mediawiki) — `pong` with nonce.
  - [BIP-37](https://github.com/bitcoin/bips/blob/master/bip-0037.mediawiki) — Bloom filter.
  - [BIP-130](https://github.com/bitcoin/bips/blob/master/bip-0130.mediawiki) — `sendheaders`.
  - [BIP-133](https://github.com/bitcoin/bips/blob/master/bip-0133.mediawiki) — `feefilter`.
  - [BIP-152](https://github.com/bitcoin/bips/blob/master/bip-0152.mediawiki) — compact block relay.
  - [BIP-155](https://github.com/bitcoin/bips/blob/master/bip-0155.mediawiki) — AddrV2.
  - [BIP-157](https://github.com/bitcoin/bips/blob/master/bip-0157.mediawiki) / [BIP-158](https://github.com/bitcoin/bips/blob/master/bip-0158.mediawiki) — compact block filters.
  - [BIP-324](https://github.com/bitcoin/bips/blob/master/bip-0324.mediawiki) — encrypted v2 transport.
  - [BIP-339](https://github.com/bitcoin/bips/blob/master/bip-0339.mediawiki) — wtxidrelay.
- libbitcoin's `network/` module — clean separation of session/protocol/channel layers.

---

## 20. Change log

| Date | Change | Author |
| --- | --- | --- |
| 2026-05-26 | Initial draft. | Claude (Opus 4.7), session-authored |
| 2026-05-26 | Added caps, keep-alive note, orphan-discard rule, seeding sequence, missing-messages section, unknown-command reconciliation, devguide-discrepancies section. | Claude (Opus 4.7), session-authored |
| 2026-05-26 | Re-framed as the application-side networking design. Added §0 lib/app split, updated §1.3 module placement, §2 layered view, §7 location note, §12.1 ownership to reflect the new constraint that no library crate touches a socket. Library-side `Transport` trait lives in technical-design §4.14. | Claude (Opus 4.7), session-authored |
