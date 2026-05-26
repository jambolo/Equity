//! General-purpose helpers used across the Equity workspace.
//!
//! The crate is small and dependency-light, providing two utility modules:
//!
//! - [`endian`] — byte-order conversions on `u16` / `u32` / `u64` via either
//!   the [`Endian`] facade or the [`EndianConvert`] trait.
//! - [`hex`] — hex encode/decode (including Bitcoin's "reversed" display-order
//!   variant) plus [`shorten_string`] for trimming long strings with an
//!   ellipsis.
//!
//! # Examples
//!
//! Round-trip a buffer through hex:
//!
//! ```
//! use utility::Hex;
//!
//! let data = vec![0xDE, 0xAD, 0xBE, 0xEF];
//! let s = Hex::encode(&data).unwrap();
//! assert_eq!(s, "deadbeef");
//! assert_eq!(Hex::decode(&s).unwrap(), data);
//! ```
//!
//! Swap byte order with the trait:
//!
//! ```
//! use utility::EndianConvert;
//!
//! assert_eq!(0x1234u16.swap_bytes(), 0x3412);
//! ```

pub mod endian;
pub mod hex;

pub use endian::{Endian, EndianConvert};
pub use hex::{FromHex, Hex, ToHex, shorten_string};
