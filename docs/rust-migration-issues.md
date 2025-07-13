# Rust Migration Issue List

This document provides a step-by-step guide for migrating the project from C++ to Rust. Each section can be converted into one or more GitHub issues to track progress.

## 1. Preliminary Research and Planning
- **Inventory current code**
  - List all libraries (`crypto`, `equity`, `network`, `p2p`, `utility`).
  - List command-line applications (`bits`, `list-prefixes`, `view-transaction`).
- **Review dependencies**
  - Identify external libraries such as OpenSSL and Boost that require Rust equivalents.
- **Define migration strategy**
  - Decide between incremental migration or a clean rewrite.

## 2. Set Up a Rust Workspace
- Create a top-level `Cargo.toml` defining a workspace.
- Add a crate for each library and application.
- Configure any cross-compilation or feature flags needed to interoperate with existing C++ code during migration.

## 3. Integrate the Build System and CI
- Update CI scripts to use `cargo build` and `cargo test`.
- Keep existing CMake builds behind a `legacy` feature while the transition is ongoing.
- Remove CMake once all components compile as Rust crates.

## 4. Port Core Libraries
### Crypto
- Rewrite cryptographic functions in Rust using crates like `openssl` or `ring`.
- Ensure unit tests match the behavior of the original C++ code.
### Utility
- Port helper utilities and use `serde`/`serde_json` for JSON handling.
### Equity Core
- Implement Bitcoin protocol logic in Rust, replacing C++ templates with idiomatic generics and ownership patterns.
### Network and P2P
- Recreate network message types and serialization using Rust structs and enums.
- Provide asynchronous networking with `tokio` or similar crates.

## 5. Port Applications and Tests
- Translate `bits`, `list-prefixes`, and `view-transaction` to Rust binaries.
- Migrate the contents of the `test/` directory to Rust's testing framework.

## 6. Interoperability and Compatibility
- If needed, create an FFI layer so remaining C++ code can call into Rust.
- Maintain compatibility of network messages and on-disk formats.

## 7. Documentation Updates
- Update the README and other docs with new build and usage instructions.
- Provide examples demonstrating the new Rust APIs.

## 8. Cleanup and Release
- Remove obsolete C++ files once all functionality has been ported.
- Tag a release marking the first fully Rust-based version of the project.
