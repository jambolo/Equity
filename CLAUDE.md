# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Context

Bitcoin protocol library + CLI apps, written in Rust. `Application` in `docs/project_organization.png` is external — not in this repo.

## Workspace Layout

```text
Cargo.toml                              # workspace root
libs/{crypto,equity,network,p2p,utility}/
    Cargo.toml
    src/lib.rs
apps/{bits,list-prefixes,view-transactions}/src/main.rs   # CLI binaries
```

Workspace pins: `edition = "2024"`, `resolver = "2"`. Shared deps in `[workspace.dependencies]`: `tokio`, `serde`, `anyhow`, `clap`.

## Build / Test

Cargo target dir: `D:/build/rust/target` (set via `.cargo/config.toml` or env — do not commit local builds to repo).

```powershell
cargo build --workspace             # all crates
cargo build -p equity               # single crate
cargo test --lib -p utility         # single crate's #[test] modules
cargo test --workspace              # all crates
```

`secp256k1-sys` vendors `libsecp256k1`, so no system C crypto library is needed.

### Current build/test state

| Crate | build | test |
| --- | --- | --- |
| utility | OK | 18/18 |
| p2p | OK | 5/5 |
| crypto | OK | 33/33 |
| equity | OK | 74/74 |
| network | OK | 25/25 |

## Architecture Notes

### Crate roles

| Crate | Purpose |
| --- | --- |
| `utility` | Endian conversions (intrinsic-backed), hex encode/decode (`hex` crate), `shorten_string`, debug helpers |
| `crypto` | SHA-1/2/256/512, RIPEMD-160, HMAC, PBKDF2, secp256k1 ECC, OS-RNG (`getrandom`) |
| `p2p` | Wire-format primitives: CompactSize/varint, var-string, minimal `Message` envelope |
| `network` | Bitcoin P2P message types and envelope (`Header`, `Address`, `InventoryId`, `Message` enum with 22 variants, `WireMessage` with double-SHA256 checksum) |
| `equity` | Bitcoin core: addresses, keys, scripts, transactions, blocks, merkle tree, mnemonic |

## Conventions

- Prefer well-known crates over reimplementing primitives (`hmac` + `sha2`, `secp256k1`, `bs58`, `serde_json`, `getrandom`, `hex`).
- Tests: inline `#[cfg(test)] mod tests` in the crate that owns the type.

## Code Style

Standard Rust 2024 idioms. `anyhow::Result` for fallible APIs at crate boundaries; concrete error types only where callers need to discriminate. No `unsafe`. Hex via the `hex` crate, not handrolled.
