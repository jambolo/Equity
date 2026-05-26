# Equity — Current Status

Snapshot: 2026-05-25. Branch: `develop`.

## Purpose

Bitcoin protocol library + CLI apps. Mid-migration C++ → Rust via `cxx` bridges. Hybrid build.

## Layout

```text
Equity/
├── Cargo.toml           # workspace: libs/{crypto,equity,network,p2p,utility} + apps/{bits,list-prefixes,view-transactions}
├── CMakeLists.txt       # legacy C++ build (still present)
├── crypto/  equity/  network/  p2p/  utility/   # C++ sources
├── libs/<name>/src/     # Rust crate + <name>_wrapper.{h,cpp} cxx shim
├── apps/<name>/src/main.rs   # CLI stubs (hello-world only)
├── test/                # C++ gtest suite (not yet ported)
└── docs/
    ├── rust-migration-issues.md     # plan (checkboxes updated)
    └── current-project-status.md    # this file
```

## Migration phase

Phase: all hybrid tests pass. **128/128 across the workspace, zero aborts, zero failures.** WolfSSL fully dropped — all crypto primitives are pure Rust (`sha1`, `sha2`, `ripemd`, `hmac`, `pbkdf2`, `secp256k1`, `bs58` crates + `std::random_device` for entropy). The remaining C++ surface is non-crypto (equity address/transaction/block/script/mnemonic/merkle-tree, network message parsing) and is wrapped by cxx bridges that work end-to-end.

Recent session changes (uncommitted):

- All crypto C++ files reduced to thin shims delegating to Rust via `extern "C"` exports in `libs/crypto/src/{ecc_ffi,hash_ffi}.rs`. `crypto/{Sha1,Sha256,Sha512,Ripemd,Pbkdf2,Ecc}.cpp` now ~20 lines each.
- `crypto/Random.cpp` deleted entirely; random funcs implemented inline in `libs/crypto/src/crypto_wrapper.cpp` via `std::random_device`.
- `crypto/Hmac.{cpp,h}` deleted (Rust-only via `hmac` + `sha2`).
- `equity/Base58.cpp` reimplemented in plain C++ (was `NOT_YET_IMPLEMENTED()` stub); Rust path uses `bs58` crate.
- `libs/equity/src/equity_wrapper.cpp` merkle + mnemonic stubs replaced with real `Equity::MerkleTree` and `Equity::Mnemonic` calls.
- `network/Configuration.cpp` qualifier fix (`Network::Configuration::instance_`); `toJson` definitions added to PingMessage/PongMessage/VerackMessage/SendHeadersMessage; utility sources added to `libs/network/build.rs`.
- WolfSSL dropped: `WOLFSSL_ROOT` env var, `wolfssl` link, Windows MSVC system libs (`advapi32`/`user32`/`ws2_32` from `libs/crypto/build.rs`) all removed.
- Legacy gtest files for ported modules deleted from `test/crypto/`.
- Equity test fixtures updated: BIP-39 mnemonic validation, merkle proof verification, txid hex conversion non-palindromic input.

## Build / test matrix (Windows, `cargo`)

| Crate | `cargo build` | `cargo test --lib` |
| --- | --- | --- |
| utility | OK | 21/21 pass |
| p2p | OK | 5/5 pass |
| crypto | OK | 36/36 pass |
| network | OK | 23/23 pass |
| equity | OK | 43/43 pass |

`cargo build --workspace` finishes clean. `cargo test --workspace` is green.

## Architecture notes

### Rust→C++ FFI for cross-crate symbols

When a C++ TU in equity's bridge (e.g. `equity/PublicKey.cpp`, `equity/Mnemonic.cpp`, `equity/ScriptEngine.cpp`) calls a function declared in `crypto/<X>.h`, that function is now implemented in Rust:

1. Rust impl lives in `libs/crypto/src/{ecc,sha1,sha256,sha512,ripemd,hmac,pbkdf2}.rs`.
2. `libs/crypto/src/{ecc_ffi,hash_ffi}.rs` expose each impl as `#[unsafe(no_mangle)] pub unsafe extern "C" fn crypto_<name>(...)`.
3. `crypto/<X>.cpp` is a thin shim: declares the `extern "C"` Rust symbol and forwards to it inside the `Crypto::` namespace.
4. `equity/build.rs` still compiles `crypto/<X>.cpp` (and `crypto/Sha256.cpp`, etc.) so equity's C++ code links against the shim's symbols.
5. `libs/equity/src/lib.rs` holds a `#[used] static _CRYPTO_FFI_LINK_FORCE: [ExternFn; 13]` keepalive that names every Rust-side `extern "C"` symbol. Without this, rustc DCEs the rlib-level `#[used]` static and the linker can't resolve the symbols from the C++ side.

When adding a new `crypto::*_ffi` export, extend the equity keepalive array.

### Apps not wired

All three CLI stubs are `clap` hello-worlds ([apps/bits/src/main.rs](apps/bits/src/main.rs), [apps/list-prefixes/src/main.rs](apps/list-prefixes/src/main.rs), [apps/view-transactions/src/main.rs](apps/view-transactions/src/main.rs)). Original C++ behavior not ported.

### C++ tests not migrated

[test/](test/) holds remaining gtest `.cpp` files (TestAddress, TestTransaction, TestScript, TestBlock, etc.). Ported modules' test files (`TestHmac`, `TestSha1`, `TestSha256`, `TestSha512`, `TestRipemd`, `TestPbkdf2`, `TestEcc`, `TestRandom`, `TestBase58`, `TestBase58Check`) have been deleted in favor of inline `#[cfg(test)] mod tests` in the corresponding Rust modules.

## Key file references

- Workspace: [Cargo.toml](../Cargo.toml)
- Rust→C++ FFI exports: [libs/crypto/src/ecc_ffi.rs](../libs/crypto/src/ecc_ffi.rs), [libs/crypto/src/hash_ffi.rs](../libs/crypto/src/hash_ffi.rs)
- Equity FFI keepalive: [libs/equity/src/lib.rs](../libs/equity/src/lib.rs)
- Equity bridge wrapper: [libs/equity/src/equity_wrapper.cpp](../libs/equity/src/equity_wrapper.cpp)
- Migration plan: [rust-migration-issues.md](rust-migration-issues.md)

## Suggested next actions (priority order)

1. Port remaining gtest cases under [test/](../test/) (`TestAddress`, `TestTransaction`, `TestScript`, etc.) into `#[cfg(test)] mod tests` modules of the corresponding Rust crates, or delete them.
2. Implement real CLI behavior in `apps/*/main.rs` against the Rust lib APIs.
3. Continue porting equity C++ subsystems to Rust (Address, PrivateKey, PublicKey, Transaction, Block, Script, Mnemonic, MerkleTree) so equity's `build.rs` can drop the corresponding `.cpp` files.
4. Once all equity C++ is ported, the keepalive table in `libs/equity/src/lib.rs` can be deleted along with the `crypto/*.cpp` shims and their `extern "C"` Rust counterparts.
5. Update CI to use `cargo build --workspace` and `cargo test --workspace`.

## Toolchain / env notes

- Cargo target dir: `D:/build/rust/target` (set via `.cargo/config.toml`)
- nlohmann_json: `C:/Users/John/Projects/3rdParty/nlohmann_json/include` (override with `NLOHMANN_ROOT` env var)
- No system crypto library required — `secp256k1-sys` vendors `libsecp256k1`.
- Windows MSVC linker; cxx 1.0; cxx-build 1.0.
- `cxx-bridge` feature (default on per C++-linked crate) gates `build.rs` C++ compilation; build with `--no-default-features` for Rust-only paths — note that bridge declarations still reference cxx-generated symbols, so `--no-default-features` only works for crates whose Rust modules don't go through `crate::ffi`.
