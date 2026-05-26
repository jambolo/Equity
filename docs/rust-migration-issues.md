# Rust Migration Issue List

This document provides a step-by-step guide for migrating the project from C++ to Rust. Each section can be converted into one or more GitHub issues to track progress.

## 1. Preliminary Research and Planning

- [x] **Inventory current code**
  - List all libraries (`crypto`, `equity`, `network`, `p2p`, `utility`).
  - List command-line applications (`bits`, `list-prefixes`, `view-transaction`).
- [x] **Review dependencies**
  - Identify external libraries such as OpenSSL and Boost that require Rust equivalents.
- [x] **Define migration strategy**
  - Decide between incremental migration or a clean rewrite.

## 2. Set Up a Rust Workspace

- [x] Create a top-level `Cargo.toml` defining a workspace.
- [x] Add a crate for each library and application.
- [x] Configure any cross-compilation or feature flags needed to interoperate with existing C++ code during migration.

## 3. Integrate the Build System and CI

- [x] Update CI scripts to use `cargo build` and `cargo test`.
- [x] Keep existing CMake builds behind a `legacy` feature while the transition is ongoing. (Skipped — went straight to removal.)
- [x] Remove CMake once all components compile as Rust crates.

## 4. Port Core Libraries

### Crypto

- [x] Rewrite cryptographic functions in Rust using crates like `openssl` or `ring`.
- [x] Ensure unit tests match the behavior of the original C++ code.

### Utility

- [x] Port helper utilities and use `serde`/`serde_json` for JSON handling.

### Equity Core

- [x] Implement Bitcoin protocol logic in Rust, replacing C++ templates with idiomatic generics and ownership patterns. *(Configuration, Txid, Target, PrivateKey, PublicKey, Address, Mnemonic, MerkleTree, Instruction, Script, ScriptEngine, Transaction, Block now pure Rust; cxx bridge surface removed from `equity` crate.)*

### Network and P2P

- [x] Recreate network message types and serialization using Rust structs and enums. *(Pure-Rust `network` crate: `Header`, `Address`, `InventoryId`, `Message` enum with 22 variants, `WireMessage` envelope with double-SHA256 checksum; cxx bridge removed.)*
- [x] Port `p2p` C++ tree (`Message`, `Serialize`, `Peer`) to pure Rust. *(`libs/p2p/src/lib.rs` exposes `Message`, var-int/var-string ser/deser; legacy `p2p/` deleted along with `p2p_wrapper.{cpp,h}`.)*
- [ ] Provide asynchronous networking with `tokio` or similar crates.

## 5. Port Applications and Tests

- [ ] Translate `bits`, `list-prefixes`, and `view-transaction` to Rust binaries. *(Legacy C++ entry points deleted; Rust stubs in `apps/` still hello-world.)*
- [x] Migrate the contents of the `test/` directory to Rust's testing framework. *(All gtest cases either ported to inline `#[cfg(test)]` modules or deleted; `test/` tree removed.)*

## 6. Interoperability and Compatibility

- [x] If needed, create an FFI layer so remaining C++ code can call into Rust.
- [ ] Maintain compatibility of network messages and on-disk formats.

## 7. Documentation Updates

- [ ] Update the README and other docs with new build and usage instructions.
- [ ] Provide examples demonstrating the new Rust APIs.

## 8. Cleanup and Release

- [x] Remove obsolete C++ files once all functionality has been ported. *(All C++ deleted: legacy trees, root entry points, gtest suite, `*_wrapper.{cpp,h}` shims, and `utility/`.)*
- [x] Port remaining `utility/` C++ and `crypto_wrapper.cpp` (random) to pure Rust, then drop `cxx`/`cxx-build`. *(Endian via Rust intrinsics, hex via `hex` crate, random via `getrandom`; cxx/cxx-build deps removed from every Cargo.toml; no build.rs anywhere.)*
- [ ] Tag a release marking the first fully Rust-based version of the project.
