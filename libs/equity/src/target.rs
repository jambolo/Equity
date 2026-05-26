//! Bitcoin difficulty target — 256-bit value in either compact (`nBits`) form
//! or full 32-byte hash form.
//!
//! Compact layout (LSB → MSB):
//!   bits  0..23 — mantissa (unsigned, 23-bit)
//!   bit      23 — sign (always 0 in Bitcoin headers)
//!   bits 24..31 — exponent
//!
//! Value = mantissa * 256^(exponent - 3).

pub const TARGET_HASH_SIZE: usize = 32;
pub const DIFFICULTY_1_COMPACT: u32 = 0x1d00ffff;
pub const TARGET_0_COMPACT: u32 = 0x0100ffff;

const MANTISSA_MASK: u32 = 0x007f_ffff;
const EXPONENT_SHIFT: u32 = 24;

pub const DIFFICULTY_1_HASH: [u8; TARGET_HASH_SIZE] = {
    let mut h = [0u8; TARGET_HASH_SIZE];
    h[4] = 0xFF;
    h[5] = 0xFF;
    h
};

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Target {
    hash: [u8; TARGET_HASH_SIZE],
    compact: u32,
}

impl Target {
    pub fn from_hash(hash: [u8; TARGET_HASH_SIZE]) -> Self {
        let compact = convert_to_compact(&hash);
        Self { hash, compact }
    }

    pub fn from_compact(compact: u32) -> Self {
        let hash = convert_to_hash(compact);
        Self {
            hash,
            compact: compact.max(TARGET_0_COMPACT),
        }
    }

    pub fn hash(&self) -> &[u8; TARGET_HASH_SIZE] {
        &self.hash
    }

    pub fn compact(&self) -> u32 {
        self.compact
    }

    pub fn difficulty(&self) -> f64 {
        compact_to_double(DIFFICULTY_1_COMPACT) / compact_to_double(self.compact)
    }
}

impl PartialOrd for Target {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Target {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering {
        self.compact.cmp(&other.compact)
    }
}

fn mantissa_of(x: u32) -> u32 {
    x & MANTISSA_MASK
}

fn exponent_of(x: u32) -> i32 {
    ((x >> EXPONENT_SHIFT) & 0xff) as i32
}

pub fn convert_to_hash(compact: u32) -> [u8; TARGET_HASH_SIZE] {
    debug_assert!(compact & 0x0080_0000 == 0, "negative mantissa unsupported");

    let mut mantissa = mantissa_of(compact);
    let mut exponent = exponent_of(compact);

    if mantissa & 0x00ff_0000 == 0 {
        mantissa <<= 8;
        exponent -= 1;
    }
    debug_assert!((0..=32).contains(&exponent));

    let mut out = [0u8; TARGET_HASH_SIZE];
    let sz = TARGET_HASH_SIZE as i32;
    if exponent >= 3 {
        out[(sz + 0 - exponent) as usize] = ((mantissa >> 16) & 0xff) as u8;
        out[(sz + 1 - exponent) as usize] = ((mantissa >> 8) & 0xff) as u8;
        out[(sz + 2 - exponent) as usize] = (mantissa & 0xff) as u8;
    } else {
        if exponent >= 1 {
            out[(sz + 0 - exponent) as usize] = ((mantissa >> 16) & 0xff) as u8;
        }
        if exponent >= 2 {
            out[(sz + 1 - exponent) as usize] = ((mantissa >> 8) & 0xff) as u8;
        }
    }
    out
}

pub fn convert_to_compact(hash: &[u8; TARGET_HASH_SIZE]) -> u32 {
    let zeros = hash.iter().take_while(|b| **b == 0).count();
    if zeros == TARGET_HASH_SIZE {
        return TARGET_0_COMPACT;
    }

    let m0 = *hash.get(zeros).unwrap_or(&0) as u32;
    let m1 = *hash.get(zeros + 1).unwrap_or(&0) as u32;
    let m2 = *hash.get(zeros + 2).unwrap_or(&0) as u32;
    let mut mantissa = (m0 << 16) | (m1 << 8) | m2;
    let mut exponent = (TARGET_HASH_SIZE - zeros) as u32;

    if mantissa >= 0x0080_0000 {
        mantissa >>= 8;
        exponent += 1;
    }

    (exponent << EXPONENT_SHIFT) | mantissa
}

fn compact_to_double(compact: u32) -> f64 {
    let mantissa = mantissa_of(compact) as f64;
    let exponent = exponent_of(compact);
    mantissa * 256f64.powi(exponent - 3)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_difficulty_one_round_trip() {
        let t = Target::from_compact(DIFFICULTY_1_COMPACT);
        assert_eq!(t.compact(), DIFFICULTY_1_COMPACT);
        assert_eq!(t.hash(), &DIFFICULTY_1_HASH);
        assert!((t.difficulty() - 1.0).abs() < 1e-9);
    }

    #[test]
    fn test_compact_round_trip_normalized() {
        // Normalized targets round-trip exactly.
        for compact in [0x1d00ffff_u32, 0x1b0404cb, 0x1c2ac4af, 0x1903a30c] {
            let t = Target::from_compact(compact);
            let back = convert_to_compact(t.hash());
            assert_eq!(back, compact, "{compact:#x} round-trip");
        }
    }

    #[test]
    fn test_target_0_compact_round_trip() {
        // TARGET_0_COMPACT is the smallest representable normalized compact.
        // Sub-TARGET_0 values are not well-defined in convert_to_hash.
        let t = Target::from_compact(TARGET_0_COMPACT);
        assert_eq!(t.compact(), TARGET_0_COMPACT);
    }

    #[test]
    fn test_difficulty_strictly_increases_as_target_drops() {
        let easy = Target::from_compact(0x1d00ffff);
        let hard = Target::from_compact(0x1b0404cb); // ~7.67M difficulty
        assert!(hard.difficulty() > easy.difficulty());
    }

    #[test]
    fn test_ordering_by_compact() {
        let easier = Target::from_compact(0x1d00ffff);
        let harder = Target::from_compact(0x1b0404cb);
        assert!(harder < easier);
    }
}
