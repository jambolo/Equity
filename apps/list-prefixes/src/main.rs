//! Enumerates Base58Check encodings of the all-zero and all-FF 20- and 32-byte hashes across every
//! version byte (0..=255).

use equity::base58_check;

fn main() {
    let hash_low_20 = [0u8; 20];
    let hash_high_20 = [0xffu8; 20];
    for i in 0u8..=255 {
        let low = base58_check::encode(&hash_low_20, i);
        let high = base58_check::encode(&hash_high_20, i);
        println!("0x{i:02x}: low: {low:<35} high: {high:<35}");
    }

    let hash_low_32 = [0u8; 32];
    let hash_high_32 = [0xffu8; 32];
    for i in 0u8..=255 {
        let low = base58_check::encode(&hash_low_32, i);
        let high = base58_check::encode(&hash_high_32, i);
        println!("0x{i:02x}: low: {low:<51} high: {high:<51}");
    }
}
