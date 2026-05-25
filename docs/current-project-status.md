# Equity — Current Status

Snapshot: 2026-05-25. Branch: `develop`.

## Purpose

Bitcoin protocol library + CLI apps. Mid-migration C++ → Rust via `cxx` bridges. Hybrid build.

## Layout

```text
Equity/
├── Cargo.toml           # workspace: libs/{crypto,equity,network,p2p,utility} + apps/{bits,list-prefixes,view-transactions}
├── CMakeLists.txt       # legacy C++ build (still present)
├── crypto/  equity/  network/  p2p/  utility/   # C++ sources (~7350 LOC, 106 files)
├── libs/<name>/src/     # Rust crate + <name>_wrapper.{h,cpp} cxx shim
├── apps/<name>/src/main.rs   # CLI stubs (hello-world only)
├── test/                # C++ gtest suite (not yet ported)
└── docs/
    ├── rust-migration-issues.md       # plan (checkboxes updated)
    └── current-project-status.md     # this file
```

Rust LOC: ~7808. C++ LOC: ~7350.

## Migration phase

Recent commits (newest first):

```text
def1ca8 Port unit tests wip
d67c7f6 All libraries compile with code to bridge to C++ implementation
7ce5f86 Removed circular references in CPP code
557198e Bridged all libraries except p2p and network with cxx
6629756 Initial creation of Rust project with placeholder functionality
```

Plus untracked session work:

- `cxx-bridge` feature flag added to all C++-linked lib crates (`default = ["cxx-bridge"]`)
- `equity/build.rs` rewritten: builds from C++ source (was checking for pre-built .lib)
- `equity` cxx bridge: fixed namespace mismatches, type conflicts, merkle tree API
- `network/build.rs` fixed: added missing p2p sources (`p2p/Message.cpp`, `Peer.cpp`, `Serialize.cpp`)
- External lib paths (wolfssl, nlohmann) use `WOLFSSL_ROOT`/`NLOHMANN_ROOT` env vars with fallbacks
- All Rust compiler warnings eliminated

Phase: cxx bridges working for all libs. Unit test port WIP. Apps untouched.

## Build / test matrix (Windows, `cargo`)

| Crate | `cargo build` | `cargo test --lib` | Notes |
| --- | --- | --- | --- |
| utility | OK | 21/21 pass | bridge works; C++ utility lib linked |
| p2p | OK | 5/5 pass | Rust-only paths exercised |
| crypto | OK | runtime abort | `Assertion failed: !"Function was called but is not implemented.", file ../../crypto/Hmac.cpp, line 18` → STATUS_STACK_BUFFER_OVERRUN |
| equity | OK | runtime abort | `Assertion failed: !"Function was called but is not implemented.", file ../../equity/Base58.cpp, line 75` → STATUS_STACK_BUFFER_OVERRUN |
| network | OK | link fail | `Utility::toHex` unresolved + missing vtable entries for `Message::toJson` overrides |

`cargo build --workspace` finishes clean (no warnings).

## Blocker root causes

### crypto runtime abort

C++ `Hmac.cpp:18` is a `not implemented` stub. Rust test calls into it via bridge.

Fix: implement Hmac in Rust (`hmac` + `sha2` crates) OR replace C++ stub with real impl OR gate test behind `cfg`.

### equity runtime abort

C++ `Base58.cpp:75` is a `not implemented` stub. Rust test calls into it via bridge.

Fix: implement Base58 in Rust OR replace C++ stub with real impl.

### network test link failure

`network.lib` calls `Utility::toHex` (from utility C++ sources) and virtual `Message::toJson` overrides. Neither is compiled into the network test binary.

Fix: add utility C++ sources to network's `build.rs` (same pattern used to fix equity). Also audit which message `.cpp` files define the missing vtable symbols.

### apps not wired

All three are `clap` hello-world stubs ([apps/bits/src/main.rs](apps/bits/src/main.rs), [apps/list-prefixes/src/main.rs](apps/list-prefixes/src/main.rs), [apps/view-transactions/src/main.rs](apps/view-transactions/src/main.rs)). Original C++ behavior not ported.

### C++ tests not migrated

[test/](test/) holds gtest `.cpp` files. Inline `#[test]` modules exist in lib crates (~199 across 40 files) but coverage spotty.

## Key file references

- Workspace: [Cargo.toml](Cargo.toml)
- Migration plan: [docs/rust-migration-issues.md](docs/rust-migration-issues.md)
- equity bridge: [libs/equity/src/lib.rs](libs/equity/src/lib.rs)
- equity build: [libs/equity/build.rs](libs/equity/build.rs)
- equity C++ wrapper: [libs/equity/src/equity_wrapper.h](libs/equity/src/equity_wrapper.h)
- network build (p2p sources added): [libs/network/build.rs](libs/network/build.rs)
- utility bridge (working reference): [libs/utility/src/lib.rs](libs/utility/src/lib.rs)

## Suggested next actions (priority order)

1. Fix network test link: add `../../utility/Utility.cpp` and `../../utility/Endian.cpp` to [libs/network/build.rs](libs/network/build.rs); audit missing `Message::toJson` vtable symbols.
2. Replace `Hmac.cpp:18` stub OR move crypto Hmac to pure Rust (`hmac` + `sha2` crates).
3. Replace `Base58.cpp:75` stub OR implement Base58 in Rust.
4. Port remaining gtest cases under [test/](test/) into `#[cfg(test)]` modules of respective Rust crates.
5. Implement real CLI behavior in `apps/*/main.rs` against Rust lib APIs.
6. Update CI to use `cargo build` and `cargo test`.

## Toolchain / env notes

- Cargo target dir: `D:/build/rust/target`
- wolfssl: `C:/Users/John/Projects/3rdParty/wolfssl` (override with `WOLFSSL_ROOT` env var)
- nlohmann_json: `C:/Users/John/Projects/3rdParty/nlohmann_json/include` (override with `NLOHMANN_ROOT` env var)
- WolfSSL used for crypto C++ side.
- Windows MSVC linker; cxx 1.0; cxx-build 1.0.
- `cxx-bridge` feature (default on) gates all C++ compilation; build with `--no-default-features` for Rust-only.
