# Equity

[![CI](https://github.com/jambolo/Equity/actions/workflows/ci.yml/badge.svg)](https://github.com/jambolo/Equity/actions/workflows/ci.yml)
[![Coverage](https://codecov.io/gh/jambolo/Equity/branch/develop/graph/badge.svg)](https://codecov.io/gh/jambolo/Equity)

Bitcoin protocol library and related command-line applications, written in Rust.

The goal is to implement a Bitcoin node from scratch as a learning exercise. This repository contains the reusable protocol crates plus a few small CLI tools built on top of them.

![Project Organization](./docs/project_organization.png?raw=true)

## Workspace Layout

```text
Cargo.toml                                # workspace root
libs/{crypto,equity,network,p2p,utility}/ # protocol crates
apps/{bits,list-prefixes,view-transactions}/ # CLI binaries
docs/                                     # diagrams
```

Workspace pins: `edition = "2024"`, `resolver = "2"`. Shared dependencies (`tokio`, `serde`, `anyhow`, `clap`, `proptest`) live in `[workspace.dependencies]`.

## Libraries

| Crate | Purpose |
| --- | --- |
| `crypto` | SHA-1/2/256/512, RIPEMD-160, HMAC, PBKDF2, secp256k1 ECC, OS RNG |
| `equity` | Bitcoin core: addresses, keys, scripts, transactions, blocks, merkle tree, BIP-39 mnemonics |
| `network` | Bitcoin P2P message types and `WireMessage` envelope with double-SHA256 checksum |
| `p2p` | Wire-format primitives: CompactSize/varint, var-string, minimal `Message` envelope |
| `utility` | Endian conversions, hex encode/decode, `shorten_string`, debug helpers |

## Applications

| Binary | Purpose |
| --- | --- |
| `bits` | Decode a Bitcoin compact-target (`nBits`) value into its 256-bit hash and difficulty |
| `list-prefixes` | Enumerate Base58Check encodings of all-zero / all-FF 20- and 32-byte hashes for every version byte |
| `view-transactions` | Decode a raw transaction hex string and pretty-print its fields as JSON |

## Prerequisites

- Rust toolchain with edition 2024 support (stable `1.85+`). Install via [rustup](https://rustup.rs/).
- No system C cryptography library required — `secp256k1-sys` vendors `libsecp256k1` and randomness is provided by `getrandom`.

## Build

```powershell
cargo build --workspace          # all crates and binaries
cargo build -p equity            # one crate
cargo build --release            # optimized build
```

By default the workspace writes build artifacts to `D:/build/rust/target`. Override by editing `.cargo/config.toml` or setting `CARGO_TARGET_DIR`:

```powershell
$env:CARGO_TARGET_DIR = "target"
cargo build --workspace
```

## Test

```powershell
cargo test --workspace           # full suite
cargo test --lib -p utility      # one crate's #[test] modules
```

Current state:

| Crate | build | test |
| --- | --- | --- |
| utility | OK | 18/18 |
| p2p | OK | 5/5 |
| crypto | OK | 33/33 |
| equity | OK | 74/74 |
| network | OK | 25/25 |

## Usage

Run a binary either through Cargo or directly from the target directory.

### bits — decode a compact target

```powershell
cargo run -p bits -- 486604799
```

```text
Bits = 486604799 (0x1d00ffff)
Hash = 00000000ffff0000000000000000000000000000000000000000000000000000
Difficulty = 1
```

### view-transactions — pretty-print a raw transaction

```powershell
cargo run -p view-transactions -- <raw-tx-hex>
```

Outputs a JSON document with `version`, `inputs[]`, `outputs[]`, and `locktime`. Exits non-zero on invalid hex or malformed transaction data.

### list-prefixes — enumerate Base58Check version-byte ranges

```powershell
cargo run -p list-prefixes
```

Prints, for each version byte `0x00..0xff`, the Base58Check encoding of the all-zero and all-FF 20- and 32-byte payloads. Useful for sanity-checking address prefix tables.

## Library usage

```rust
use equity::{Network, address::Address, private_key::PrivateKey, public_key::PublicKey};

let mut k = [0u8; 32];
k[31] = 1;
let sk = PrivateKey::from_data(&k).unwrap();
let pk = PublicKey::from_private_key(&sk).unwrap();
let addr = Address::from_public_key(&pk).unwrap();
println!("{}", addr.to_string(Network::Mainnet));
```

Add a workspace crate as a dependency from another local crate:

```toml
[dependencies]
equity = { path = "../equity" }
```

Generate API documentation:

```powershell
cargo doc --workspace --no-deps --open
```

## License

MIT. See [LICENSE](LICENSE).
