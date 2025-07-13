# Rust Migration Issue List

This document outlines a possible breakdown of GitHub issues for converting the entire project from C++ to Rust. Each issue represents a milestone or set of tasks that can be tracked separately.

## 1. Project Setup and Planning
- **Define migration goals and scope**
  - Decide which components need direct translation and which can be redesigned.
- **Inventory existing C++ modules**
  - Document all libraries (`crypto`, `equity`, `network`, `p2p`, `utility`) and applications (e.g., `bits`, `list-prefixes`, `view-transaction`).
- **Establish a Rust workspace**
  - Create Cargo workspace layout mirroring current project structure.

## 2. Infrastructure
- **Build system transition**
  - Replace CMake-based builds with Cargo for Rust crates.
  - Keep optional C++ builds (if any) behind feature flags during transition.
- **Continuous Integration updates**
  - Update CI scripts to compile and run tests for Rust crates.

## 3. Library Migration
- **Crypto Library**
  - Port cryptographic functions from `crypto` to Rust (e.g., wrappers around OpenSSL or pure-Rust equivalents).
  - Write Rust unit tests to match existing C++ tests.
- **Utility Library**
  - Reimplement helper utilities in Rust.
  - Ensure JSON handling uses `serde_json`.
- **Equity Core Library**
  - Translate Bitcoin protocol logic (scripts, validators, etc.).
  - Replace C++ templates and smart pointers with idiomatic Rust generics and ownership model.
- **Network and P2P Layers**
  - Implement network message types and serialization using Rust structs and enums.
  - Provide async networking using `tokio` or similar crates.

## 4. Application Migration
- **Command-line tools**
  - Port `bits`, `list-prefixes`, and `view-transaction` to Rust binaries under the workspace.
- **Testing utilities**
  - Migrate unit tests under `test/` to Rust's built-in test framework or `cargo test`.

## 5. Interoperability and Compatibility
- **FFI bridge (optional)**
  - If gradual migration is required, create Rust bindings callable from existing C++ code.
- **Data compatibility**
  - Ensure network messages and on-disk formats remain compatible with Bitcoin standards.

## 6. Documentation and Examples
- **Update README and docs**
  - Document how to build and run the new Rust code.
  - Provide examples for using the Rust libraries.

## 7. Cleanup
- **Remove obsolete C++ code**
  - After successful migration, delete remaining C++ modules.
- **Finalize release**
  - Tag a version marking the first all-Rust release.

Each bullet can be turned into a separate GitHub issue or broken down further depending on team workflow. Tracking progress with these issues will help coordinate the conversion from C++ to Rust while maintaining project functionality.
