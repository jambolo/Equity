# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Context

Bitcoin protocol library + CLI apps. Mid-migration C++ → Rust via `cxx` bridges. Hybrid build active: Rust crates link C++ object code through cxx-build. Original C++ tree still present at repo root (`crypto/`, `equity/`, `network/`, `p2p/`, `utility/`, `test/`); legacy CMake build removed — Cargo is the only build system. `Application` in `docs/project_organization.png` is external — not in this repo.

See [docs/current-project-status.md](docs/current-project-status.md) for migration phase, build/test matrix, and known blockers. See [docs/rust-migration-issues.md](docs/rust-migration-issues.md) for the migration plan with checkbox state.

## Workspace Layout

```text
Cargo.toml                       # workspace root
libs/{crypto,equity,network,p2p,utility}/
    Cargo.toml
    build.rs                     # compiles C++ sources + cxx bridge
    src/lib.rs                   # cxx::bridge mod ffi { ... }
    src/<name>_wrapper.{h,cpp}   # C++ shim exposing C-style API to cxx
apps/{bits,list-prefixes,view-transactions}/src/main.rs   # CLI stubs (not ported)
crypto/ equity/ network/ p2p/ utility/   # C++ sources, compiled by build.rs
test/                            # C++ gtest suite, not yet ported
```

Workspace pins: `edition = "2024"`, `resolver = "2"`. Shared deps in `[workspace.dependencies]`: `tokio`, `serde`, `anyhow`, `clap`. `cxx = "1.0"` declared per-crate.

## Build / Test

Cargo target dir: `D:/build/rust/target` (set via `.cargo/config.toml` or env — do not commit local builds to repo).

```powershell
cargo build --workspace             # all crates, cxx-bridge default ON
cargo build -p equity               # single crate
cargo test --lib -p utility         # single crate's #[test] modules
cargo build --no-default-features   # skip C++ compilation (Rust-only paths)
```

`cxx-bridge` is a per-crate default feature (`crypto`, `equity`, `network`, `utility`). Disabling it skips `build.rs` C++ compilation entirely. `p2p` has no such feature — it is Rust-only.

### External dependencies (env overrides)

| Var | Default | Purpose |
| --- | --- | --- |
| `NLOHMANN_ROOT` | `C:/Users/John/Projects/3rdParty/nlohmann_json/include` | nlohmann_json header-only |

WolfSSL is no longer used — all crypto primitives (sha1/sha256/sha512/ripemd160/hmac/pbkdf2/ecc/random) are pure Rust via `sha1`, `sha2`, `ripemd`, `hmac`, `pbkdf2`, `secp256k1` crates + `std::random_device`. The `secp256k1` crate vendors `libsecp256k1`, so no system C library is required for crypto.

### Current build/test state

| Crate | build | test |
| --- | --- | --- |
| utility | OK | 21/21 |
| p2p | OK | 5/5 |
| crypto | OK | 36/36 |
| equity | OK | 43/43 |
| network | OK | 23/23 |

If editing `network/build.rs`, follow the equity pattern: include both `../../utility/Utility.cpp` and `../../utility/Endian.cpp` so dependent symbols resolve.

## Architecture Notes

### cxx bridge pattern

Each lib crate exposes its C++ surface through a `#[cxx::bridge] mod ffi` in `src/lib.rs`. The bridge references types and free functions declared in `src/<name>_wrapper.h` and implemented in `src/<name>_wrapper.cpp`. The wrapper is a flat C-style API around the original C++ classes (e.g. `addressFromString(s: &str) -> AddressCpp`); cxx cannot bridge C++ classes directly, so `AddressCpp` etc. are POD mirrors of the underlying objects' state.

C++ source compilation happens in `build.rs` via `cxx_build::bridge("src/lib.rs")` chained with `.file(...)` calls. `equity/build.rs` is the reference — it pulls in equity's own `.cpp`, plus crypto and utility `.cpp` files that equity calls directly (cross-crate C++ deps don't flow through Cargo).

### Inter-crate dependencies

`equity` depends on `crypto` and `utility` (Cargo). C++ sources from those crates are compiled *into equity's bridge*, not linked from sibling crates — each crate's `build.rs` re-compiles whatever C++ TU it touches. This is intentional during migration: it avoids needing to publish a static archive between crates.

### Rust→C++ FFI for cross-crate symbols

`crypto/{Ecc,Sha1,Sha256,Sha512,Ripemd,Hmac,Pbkdf2}.cpp` are now thin C++ shims that delegate to `extern "C"` Rust exports defined in [libs/crypto/src/ecc_ffi.rs](libs/crypto/src/ecc_ffi.rs) + [libs/crypto/src/hash_ffi.rs](libs/crypto/src/hash_ffi.rs). The crypto rlib alone does not preserve those symbols across the rlib boundary, so [libs/equity/src/lib.rs](libs/equity/src/lib.rs) holds a `#[used] static` keepalive that names every `extern "C"` function it transitively depends on. Add a new entry whenever a new `crypto::*_ffi` symbol is added.

### Circular-reference history

Some C++ files used to have circular includes that the C++ compiler tolerated but cxx-bindgen / Rust would not. Commit `7ce5f86` cleaned these. When adding new bridge surface, do not reintroduce headers that include each other.

## Migration Conventions

- Default to porting logic to pure Rust where a quality crate exists (`hmac` + `sha2`, `secp256k1`, `bs58`, `serde_json`). Drop the corresponding C++ source from `build.rs` once the bridge points at Rust internals.
- When the C++ side still has callers that need the symbol (e.g. `Mnemonic.cpp` calls `Crypto::pbkdf2HmacSha512`), keep the `.cpp` file but reduce it to a thin shim calling the Rust impl via `extern "C"`. Pattern: see [crypto/Sha256.cpp](crypto/Sha256.cpp) + [libs/crypto/src/hash_ffi.rs](libs/crypto/src/hash_ffi.rs). Don't forget to extend the keepalive in [libs/equity/src/lib.rs](libs/equity/src/lib.rs).
- Until a function is ported, keep the cxx bridge stable — Rust callers should not care whether the body is C++ or Rust.
- Tests: inline `#[cfg(test)] mod tests` in the crate that owns the type. The legacy `test/` directory holds gtest cases still being translated.
- Apps in `apps/*` are `clap` hello-world stubs; original behavior lives in the C++ `apps/<name>/main.cpp` equivalents (`bits/`, `list-prefixes/`, `view-transaction/`).

## Code Style

Standard Rust 2024 idioms. `anyhow::Result` for fallible APIs at crate boundaries; concrete error types only where callers need to discriminate. No `unsafe` outside the `cxx::bridge` mod. Hex via `hex` crate, not handrolled.
