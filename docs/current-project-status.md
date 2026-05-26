# Equity — Current Status

Snapshot: 2026-05-25. Branch: `develop`.

## Purpose

Bitcoin protocol library + CLI apps. Pure-Rust workspace; C++ migration complete.

## Layout

```text
Equity/
├── Cargo.toml                # workspace: libs/{crypto,equity,network,p2p,utility} + apps/{bits,list-prefixes,view-transactions}
├── libs/<name>/src/          # pure Rust crate
├── apps/<name>/src/main.rs   # CLI stubs (hello-world only)
└── docs/
    ├── rust-migration-issues.md   # plan (checkboxes updated)
    └── current-project-status.md  # this file
```

## Migration phase

Phase: **migration complete — zero C++**. **155/155 across the workspace, zero aborts, zero failures.**

Recent session changes (uncommitted):

- Ported `utility/Endian.{cpp,h}` to pure Rust ([libs/utility/src/endian.rs](../libs/utility/src/endian.rs)) using `u16::swap_bytes`/`to_le`/`to_be` intrinsics.
- Ported `utility/Utility.{cpp,h}` (hex encode/decode/reverse, `shorten`) to pure Rust ([libs/utility/src/hex.rs](../libs/utility/src/hex.rs)) backed by the `hex` crate.
- Ported `crypto_wrapper.cpp` random RNG to pure Rust ([libs/crypto/src/random.rs](../libs/crypto/src/random.rs)) backed by `getrandom = "0.2"`.
- Removed all `cxx`/`cxx-build` deps, all `build.rs` files, all `*_wrapper.{cpp,h}` shims, and the `utility/` C++ tree.
- Dropped the per-crate `cxx-bridge` feature.

## Build / test matrix (Windows, `cargo`)

| Crate | `cargo build` | `cargo test --lib` |
| --- | --- | --- |
| utility | OK | 18/18 pass |
| p2p | OK | 5/5 pass |
| crypto | OK | 33/33 pass |
| network | OK | 25/25 pass |
| equity | OK | 74/74 pass |

`cargo build --workspace` finishes clean. `cargo test --workspace` is green.

## Architecture notes

### Crate roles

| Crate | Purpose |
| --- | --- |
| `utility` | Endian conversions, hex encode/decode, `shorten_string`, debug helpers |
| `crypto` | SHA-1/2/256/512, RIPEMD-160, HMAC, PBKDF2, secp256k1 ECC, OS RNG |
| `p2p` | Wire-format primitives: CompactSize/varint, var-string, minimal `Message` |
| `network` | Bitcoin P2P message types, `WireMessage` envelope with checksum |
| `equity` | Bitcoin core: addresses, keys, scripts, transactions, blocks, merkle, mnemonic |

### Apps not wired

All three CLI stubs are `clap` hello-worlds ([apps/bits/src/main.rs](../apps/bits/src/main.rs), [apps/list-prefixes/src/main.rs](../apps/list-prefixes/src/main.rs), [apps/view-transactions/src/main.rs](../apps/view-transactions/src/main.rs)).

## Key file references

- Workspace: [Cargo.toml](../Cargo.toml)
- Pure-Rust random: [libs/crypto/src/random.rs](../libs/crypto/src/random.rs)
- Pure-Rust endian: [libs/utility/src/endian.rs](../libs/utility/src/endian.rs)
- Pure-Rust hex / shorten: [libs/utility/src/hex.rs](../libs/utility/src/hex.rs)
- Migration plan: [rust-migration-issues.md](rust-migration-issues.md)

## Suggested next actions (priority order)

1. Update CI to use `cargo build --workspace` and `cargo test --workspace`.
2. Wire async networking (tokio) on top of `network`'s wire-format types.
3. Refresh README + add usage examples.
4. Tag a release marking the first fully Rust-based version of the project.

## Toolchain / env notes

- Cargo target dir: `D:/build/rust/target` (set via `.cargo/config.toml`)
- No system crypto library required — `secp256k1-sys` vendors `libsecp256k1`; randomness via `getrandom`.
- No `cxx`/`cxx-build` dependency anywhere in the workspace.
