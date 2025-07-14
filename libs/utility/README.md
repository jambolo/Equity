# Utility Library

This Rust library provides a comprehensive wrapper around the C++ utility functionality using the `cxx` bridge.

## Features

### Endian Conversion (`endian.rs`)
- Byte order conversions (swap, little-endian, big-endian)
- Support for 16-bit, 32-bit, and 64-bit integers
- Trait-based interface with `EndianConvert`

### Hexadecimal Operations (`hex.rs`)
- Standard hex encoding/decoding with `ToHex` and `FromHex` traits
- Bitcoin-specific reversed hex encoding (little-endian byte order)
- String shortening utilities
- Safe array conversions

### Bitcoin Merkle Trees (`merkle.rs`)
- Complete Merkle tree implementation for Bitcoin block verification
- Proof generation and verification
- Serialized operations to work within cxx constraints
- Hash utilities for Bitcoin-compatible operations

### Debug Utilities (`debug.rs`)
- Conditional compilation macros
- `not_yet_implemented!()` for unfinished code
- `this_should_never_happen!()` for unreachable code
- `debug_only!()` and `release_only!()` conditional execution

## Usage

```rust
use utility::endian::EndianConvert;
use utility::hex::{ToHex, FromHex};
use utility::merkle::MerkleTree;

// Endian conversions
let value = 0x1234u16;
let swapped = value.swap();

// Hex operations
let data = vec![0xde, 0xad, 0xbe, 0xef];
let hex_string = data.to_hex().unwrap();
let decoded = Vec::from_hex(&hex_string).unwrap();

// Merkle trees
let hashes = vec![/* 32-byte hashes */];
let tree = MerkleTree::new_serialized(&hashes).unwrap();
let root = tree.root().unwrap();
```

## C++ Dependencies

This library bridges the following C++ files:
- `Endian.cpp/h` - Byte order conversion utilities
- `MerkleTree.cpp/h` - Bitcoin Merkle tree implementation  
- `Utility.cpp/h` - Hex conversion and string utilities

Additionally includes crypto dependencies:
- All crypto library files for SHA256 operations required by MerkleTree

## Testing

Run tests with:
```bash
cargo test
```

All 29 tests pass, covering:
- Endian conversions (5 tests)
- Hex operations (6 tests) 
- Merkle tree operations (12 tests)
- Debug utilities (6 tests)
