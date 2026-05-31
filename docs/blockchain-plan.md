# blockchain crate — implementation plan

Status: **plan, not implemented**. Many details TBD; this document captures the design decisions made so far so any future agent can resume work without re-deriving context.

## Goal

Add a new workspace crate `libs/blockchain` providing two persistent stores:

1. **UTXO set** — unspent transaction outputs. Hot, frequently updated, working set ~10 GB, total tens of GB. Random-access reads dominate.
2. **Block bytes** — full serialized blocks. Cold tail, append-dominated, total **> 1 TB**. All but the most recent blocks are effectively immutable.

Must be fast and multi-thread-friendly. Backend differs from existing crates (no current crate uses RocksDB or any on-disk DB).

## Workspace context

Repo layout already established (see [CLAUDE.md](../CLAUDE.md)):

```text
Cargo.toml                                                # workspace root
libs/{crypto,equity,network,p2p,utility}/                 # existing crates
apps/{bits,list-prefixes,view-transactions}/              # existing CLI apps
```

Workspace pins: `edition = "2024"`, `resolver = "2"`. Cargo target dir set to `D:/build/rust/target` via `.cargo/config.toml`.

New crate goes at `libs/blockchain/`. Must be added to root `Cargo.toml` `[workspace]` members.

Existing types the new crate will consume:

- `equity::block::{Block, BlockHeader}` — already has `serialize` / `deserialize` taking `&mut &[u8]`
- `equity::txid::Txid` — 32-byte hash, display-order storage
- `equity::transaction::{Transaction, Input, Output}`
- `crypto::sha256` — hashing
- block / transaction hashing already handled by equity (no need to re-implement)

## Dependency additions

Add to workspace deps in root `Cargo.toml` (or directly under `libs/blockchain/Cargo.toml` if not shared):

```toml
[dependencies]
rocksdb = { version = "0.23", default-features = false, features = ["snappy", "lz4", "multi-threaded-cf"] }
anyhow.workspace = true
serde.workspace = true
bincode = "1.3"            # compact UTXO value encoding
parking_lot = "0.12"       # faster RwLock/Mutex for hot paths
directories = "5"          # OS-correct app data dir
thiserror = "1"            # crate-local error enum
```

Crate path deps: `crypto`, `equity`, `network`.

### RocksDB build notes

- `rocksdb` crate vendors C++ RocksDB via `librocksdb-sys`. First build is long (~5 min).
- Windows requires MSVC toolchain; clang not needed because we disable `zstd`/`bzip2` default features.
- Verify build green on all three OS targets before proceeding past phase 1.

## Architecture overview

Two stores, intentionally different backends. **Do not** try to put 1 TB of block bytes into RocksDB — its compactor over that volume wastes IO badly.

| Store | Backend | Rationale |
| --- | --- | --- |
| UTXO set | RocksDB column family | Random-access r/w, hot, value churn |
| Block bytes | Flat append-only files + RocksDB index | Sequential append, immutable tail, huge volume |

Both stores live under one OS-appropriate data directory (see "OS abstraction" below).

## UTXO store design

One RocksDB instance shared with the block index (column-family separated). Dedicated CF: `"utxo"`.

### Key format

`txid (32 bytes) || vout (u32 big-endian)` = 36 bytes. Big-endian on `vout` so that all outputs of a given txid form a contiguous prefix range — enables `prefix_iterator` when a transaction is fully spent.

### Value format

bincode-encoded:

```rust
#[derive(serde::Serialize, serde::Deserialize)]
pub struct Utxo {
    pub amount: u64,
    pub script: Vec<u8>,
    pub height: u32,
    pub coinbase: bool,
}
```

### CF tuning

- `BlockBasedOptions::set_block_size(16 KB)`
- Bloom filter, 10 bits/key — UTXO lookups are random, bloom kills disk hits on misses
- Compression: LZ4 (decode is fast; UTXO values are small)
- Prefix extractor: first 32 bytes (txid)
- `set_write_buffer_size(256 MB)`, allow multiple memtables

### Concurrency

- `Arc<DB>`, cloneable. `rocksdb::DB` is `Send + Sync`.
- `multi-threaded-cf` feature for parallel CF ops
- Writes per block batched into a single `WriteBatch` for atomic apply
- Reads lock-free

### API sketch

```rust
pub struct UtxoStore { /* db handle + cf */ }

impl UtxoStore {
    pub fn get(&self, txid: &Txid, vout: u32) -> Result<Option<Utxo>>;
    pub fn multi_get(&self, keys: &[(Txid, u32)]) -> Result<Vec<Option<Utxo>>>;
    pub fn apply_block(
        &self,
        spent: &[OutPoint],
        created: &[(OutPoint, Utxo)],
    ) -> Result<()>;
    pub fn flush(&self) -> Result<()>;
}
```

`OutPoint` is `(Txid, u32)`; reuse existing one from equity if present, else define in this crate.

## Block store design

Mirrors Bitcoin Core's `blk*.dat` strategy: raw block bytes in flat append-only files, RocksDB stores an index so reads are one disk seek.

### Flat files

- File names: `blk00000.dat`, `blk00001.dat`, ...
- Max ~128 MB per file. Rotate when next block would exceed cap.
- Append-only. Never modified after a block lands.

### RocksDB index CFs

CF `"block_index"`:
- key: `block_hash` (32 bytes)
- value: bincode of `{ file_num: u32, offset: u64, length: u32, height: u32, header: BlockHeader }`

CF `"height_to_hash"`:
- key: `height` (u32 big-endian)
- value: `block_hash` (32 bytes)
- Supports ranged scans by height.

CF `"meta"`:
- chain tip hash
- total chain work
- current write file num + offset
- schema version

### Write path

1. Open current `blk*.dat` (append mode), write block bytes, record offset.
2. Group fsync (not per-block — coalesce across N blocks or T ms).
3. Rotate file when threshold exceeded.
4. `WriteBatch`: block_index entry + height_to_hash entry + meta update. Atomic.

### Read path

1. `hash → block_index → (file_num, offset, length)`
2. `pread` into a buffer. File handles cached in `LruCache<u32, File>` behind `parking_lot::Mutex`.
3. Optional mmap for hot recent file (last N MB) on Linux/macOS. Windows uses a separate `MapViewOfFile` path or skips mmap.

### API sketch

```rust
pub struct BlockStore { /* dir, db, file lru */ }

impl BlockStore {
    pub fn put(&self, block: &Block, height: u32) -> Result<()>;
    pub fn get(&self, hash: &[u8; 32]) -> Result<Option<Block>>;
    pub fn get_by_height(&self, height: u32) -> Result<Option<Block>>;
    pub fn header(&self, hash: &[u8; 32]) -> Result<Option<BlockHeader>>;
    pub fn tip(&self) -> Result<Option<[u8; 32]>>;
}
```

`header()` is the key acceleration: header lives in the RocksDB index, no flat-file seek needed for chain walks / locator construction / merkle root lookup.

## OS abstraction

Data dir resolution via `directories::ProjectDirs`:

- Linux: `~/.local/share/equity/`
- macOS: `~/Library/Application Support/equity/`
- Windows: `%APPDATA%\equity\`

OS-specific I/O backend behind a trait `FileBackend`. Three impls gated by `#[cfg(target_os = ...)]`:

| Concern | Linux | macOS | Windows |
| --- | --- | --- | --- |
| Positional read | `pread` | `pread` | `ReadFile` + `OVERLAPPED` |
| mmap recent file | yes | yes | `MapViewOfFile`, separate impl |
| Durable fsync | `fdatasync` | `fcntl(F_FULLFSYNC)` | `FlushFileBuffers` |
| O_DIRECT (optional, off by default) | yes | n/a | unbuffered handle |

RocksDB internally handles its own OS abstraction — only the flat-file backend needs the cfg split.

## Concurrency model

- Single `Database` facade type owns the `Arc<DB>` and the `BlockStore`. Cloneable.
- Reads fully parallel — both RocksDB and `pread` are thread-safe.
- Writes serialized at the layer above (the chain validator owns block ordering). The store does not need internal write locks beyond what RocksDB provides.
- Async callers wrap blocking calls in `tokio::task::spawn_blocking`. RocksDB ops are synchronous syscalls. **Do not** add an async API surface inside this crate; let callers decide.

## Module layout

```text
libs/blockchain/
    Cargo.toml
    src/
        lib.rs              # re-exports + `Database` facade
        config.rs           # paths, tunables, OS detection
        error.rs            # `Error` enum via thiserror
        utxo.rs             # UtxoStore
        blocks/
            mod.rs          # BlockStore facade
            index.rs        # RocksDB index CFs
            files.rs        # flat-file writer / reader
            backend_unix.rs # cfg(unix) FileBackend impl
            backend_windows.rs # cfg(windows) FileBackend impl
```

## Phase order

Implement in this order. Each phase must compile and pass its tests on all three OSes before moving on.

1. **Crate scaffold + RocksDB green build** — empty `lib.rs`, deps declared, `cargo build` succeeds on Linux/macOS/Windows. No logic yet.
2. **`config` + paths** — `ProjectDirs` wiring, `Database::open(path)` stub that creates RocksDB with the three required CFs.
3. **`UtxoStore`** — single CF, default tuning, `get` / `multi_get` / `apply_block`. Unit tests against a tempdir.
4. **`UtxoStore` tuning + benchmarks** — bloom, prefix extractor, write buffer sizing. Criterion benchmarks for `get` / batched `apply_block`.
5. **`BlockStore` flat files + index** — append, rotate, index entries, basic read path. No OS split yet — use `std::fs::File` only.
6. **OS-specific `FileBackend` impls** — Unix `pread` / `fdatasync` / `F_FULLFSYNC` and Windows `ReadFile` / `FlushFileBuffers`. Optional mmap path.
7. **`Database` integration test** — open, write N blocks + corresponding UTXO deltas, close, reopen, verify state.
8. **Pruning + reorg** — out of scope for initial cut; tracked under "Open questions" below.

## Open questions / deferred decisions

These need product-level input before implementation. Capture answers here as they arrive.

- **Pruning policy.** Drop fully-spent UTXOs immediately, or keep until height N+depth? Drop old block bytes after archival snapshot?
- **Reorg / undo data.** UTXO rollback needs parallel `rev*.dat` undo files (Bitcoin Core convention). Skipped in phase 1–7. Reorgs of depth > 0 will be unsupported until undo data lands.
- **Snapshot / checkpoint format.** Useful for fast new-node sync. Format not designed yet.
- **Corruption recovery.** Torn writes mid-rotate must be detectable. Likely a length-prefixed framing inside `blk*.dat` plus a header magic per record. Spec not written.
- **Schema versioning.** Store a version byte in `meta` CF from day one; migration code is future work.
- **Large blocks.** `equity::Block::serialize` returns a `Vec<u8>`. For 1 GB+ future blocks, streaming serialize would be needed. Out of scope for this crate but worth flagging upstream.
- **Tokio integration.** Crate stays synchronous. If a future async caller appears, it wraps via `spawn_blocking`. Do not add a parallel async API.

## Glossary (for cold-start agents)

- **UTXO** — Unspent Transaction Output. A coin that exists but has not yet been spent. Identified by `(txid, vout)`.
- **OutPoint** — the `(txid, vout)` pair.
- **HASH160** — RIPEMD-160(SHA-256(pubkey)), 20 bytes; not used directly here but appears in scripts.
- **CF** — RocksDB Column Family. Independent keyspace within one DB; own tuning and bloom config.
- **blk\*.dat** — flat append-only file holding raw block bytes, Bitcoin Core convention.

## When resuming this plan

1. Re-read this file and [CLAUDE.md](../CLAUDE.md) before touching code.
2. Confirm phase boundary: which phase finished, which is next.
3. RocksDB build is slow — schedule the first compile somewhere it won't block iteration.
4. Open questions above are gates, not decoration. Do not implement pruning or reorg without resolving the relevant question first.
