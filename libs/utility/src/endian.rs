//! Endian conversion utilities

use crate::ffi;

/// Endian conversion operations
pub struct Endian;

impl Endian {
    /// Swap byte order of a 16-bit value
    pub fn swap16(x: u16) -> u16 {
        ffi::utilityEndianSwap16(x)
    }
    
    /// Swap byte order of a 32-bit value
    pub fn swap32(x: u32) -> u32 {
        ffi::utilityEndianSwap32(x)
    }
    
    /// Swap byte order of a 64-bit value
    pub fn swap64(x: u64) -> u64 {
        ffi::utilityEndianSwap64(x)
    }
    
    /// Convert to little-endian format (16-bit)
    pub fn to_little16(x: u16) -> u16 {
        ffi::utilityEndianLittle16(x)
    }
    
    /// Convert to little-endian format (32-bit)
    pub fn to_little32(x: u32) -> u32 {
        ffi::utilityEndianLittle32(x)
    }
    
    /// Convert to little-endian format (64-bit)
    pub fn to_little64(x: u64) -> u64 {
        ffi::utilityEndianLittle64(x)
    }
    
    /// Convert to big-endian format (16-bit)
    pub fn to_big16(x: u16) -> u16 {
        ffi::utilityEndianBig16(x)
    }
    
    /// Convert to big-endian format (32-bit)
    pub fn to_big32(x: u32) -> u32 {
        ffi::utilityEndianBig32(x)
    }
    
    /// Convert to big-endian format (64-bit)
    pub fn to_big64(x: u64) -> u64 {
        ffi::utilityEndianBig64(x)
    }
}

/// Trait for endian conversions
pub trait EndianConvert {
    /// Swap byte order
    fn swap_bytes(self) -> Self;
    /// Convert to little-endian
    fn to_little_endian(self) -> Self;
    /// Convert to big-endian
    fn to_big_endian(self) -> Self;
}

impl EndianConvert for u16 {
    fn swap_bytes(self) -> Self {
        Endian::swap16(self)
    }
    
    fn to_little_endian(self) -> Self {
        Endian::to_little16(self)
    }
    
    fn to_big_endian(self) -> Self {
        Endian::to_big16(self)
    }
}

impl EndianConvert for u32 {
    fn swap_bytes(self) -> Self {
        Endian::swap32(self)
    }
    
    fn to_little_endian(self) -> Self {
        Endian::to_little32(self)
    }
    
    fn to_big_endian(self) -> Self {
        Endian::to_big32(self)
    }
}

impl EndianConvert for u64 {
    fn swap_bytes(self) -> Self {
        Endian::swap64(self)
    }
    
    fn to_little_endian(self) -> Self {
        Endian::to_little64(self)
    }
    
    fn to_big_endian(self) -> Self {
        Endian::to_big64(self)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_swap16() {
        assert_eq!(Endian::swap16(0x1234), 0x3412);
        assert_eq!(0x1234u16.swap_bytes(), 0x3412);
    }

    #[test]
    fn test_swap32() {
        assert_eq!(Endian::swap32(0x12345678), 0x78563412);
        assert_eq!(0x12345678u32.swap_bytes(), 0x78563412);
    }

    #[test]
    fn test_swap64() {
        assert_eq!(Endian::swap64(0x123456789abcdef0), 0xf0debc9a78563412);
        assert_eq!(0x123456789abcdef0u64.swap_bytes(), 0xf0debc9a78563412);
    }

    #[test]
    fn test_endian_conversions() {
        let value = 0x1234u16;
        
        // Test that converting to the same endianness is identity on the target platform
        #[cfg(target_endian = "little")]
        {
            assert_eq!(value.to_little_endian(), value);
        }
        
        #[cfg(target_endian = "big")]
        {
            assert_eq!(value.to_big_endian(), value);
        }
    }
}
