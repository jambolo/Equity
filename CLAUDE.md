# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Context

Bitcoin protocol library + CLI apps, written in pure Rust. The C++ → Rust migration is complete: no C++ remains in the tree and the build no longer uses `cxx`/`cxx-build`. `Application` in `docs/project_organization.png` is external — not in this repo.

See [docs/current-project-status.md](docs/current-project-status.md) for build/test matrix and remaining work. See [docs/rust-migration-issues.md](docs/rust-migration-issues.md) for the migration plan with checkbox state.

## Workspace Layout

```text
Cargo.toml                              # workspace root
libs/{crypto,equity,network,p2p,utility}/
    Cargo.toml
    src/lib.rs                          # pure Rust
apps/{bits,list-prefixes,view-transactions}/src/main.rs   # CLI stubs (not ported)
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

No `build.rs`, no C++ toolchain required. `secp256k1-sys` vendors `libsecp256k1`, so no system C crypto library is needed.

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

## Migration Conventions

- Default to porting logic to pure Rust where a quality crate exists (`hmac` + `sha2`, `secp256k1`, `bs58`, `serde_json`, `getrandom`, `hex`).
- Tests: inline `#[cfg(test)] mod tests` in the crate that owns the type.
- Apps in `apps/*` are `clap` hello-world stubs; original C++ entry points have been deleted.

## Code Style

Standard Rust 2024 idioms. `anyhow::Result` for fallible APIs at crate boundaries; concrete error types only where callers need to discriminate. No `unsafe` outside the FFI export modules. Hex via the `hex` crate, not handrolled.
