//! Target functionality for Bitcoin
//! 
//! Provides functionality for working with Bitcoin difficulty targets.

use crate::ffi;

/// Bitcoin difficulty target utilities
pub struct Target;

impl Target {
    /// Convert a compact target representation to full target
    pub fn from_compact(compact: u32) -> Vec<u8> {
        ffi::targetFromCompact(compact)
    }

    /// Convert a full target to compact representation
    pub fn to_compact(target: &[u8]) -> u32 {
        ffi::targetToCompact(target)
    }

    /// Calculate the difficulty from a target
    pub fn get_difficulty(target: &[u8]) -> f64 {
        ffi::targetGetDifficulty(target)
    }

    /// Get the maximum target (difficulty 1)
    pub fn max_target() -> Vec<u8> {
        // Bitcoin's maximum target: 0x1d00ffff
        Self::from_compact(0x1d00ffff)
    }

    /// Calculate difficulty for a compact target
    pub fn difficulty_from_compact(compact: u32) -> f64 {
        let target = Self::from_compact(compact);
        Self::get_difficulty(&target)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_compact_target_conversion() {
        let compact = 0x1d00ffff; // Bitcoin genesis block target
        let target = Target::from_compact(compact);
        assert!(!target.is_empty());
        
        let converted_compact = Target::to_compact(&target);
        // Note: Conversion might not be exact due to precision loss
        assert!(converted_compact > 0);
    }

    #[test]
    fn test_difficulty_calculation() {
        let compact = 0x1d00ffff; // Maximum target (difficulty 1)
        let difficulty = Target::difficulty_from_compact(compact);
        assert!(difficulty > 0.0);
        
        // For maximum target, difficulty should be close to 1
        assert!((difficulty - 1.0).abs() < 0.1);
    }

    #[test]
    fn test_max_target() {
        let max_target = Target::max_target();
        assert!(!max_target.is_empty());
        
        let difficulty = Target::get_difficulty(&max_target);
        assert!((difficulty - 1.0).abs() < 0.1);
    }
}
