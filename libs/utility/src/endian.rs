//! Endian conversion utilities.

pub struct Endian;

impl Endian {
    pub fn swap16(x: u16) -> u16 {
        x.swap_bytes()
    }

    pub fn swap32(x: u32) -> u32 {
        x.swap_bytes()
    }

    pub fn swap64(x: u64) -> u64 {
        x.swap_bytes()
    }

    pub fn to_little16(x: u16) -> u16 {
        x.to_le()
    }

    pub fn to_little32(x: u32) -> u32 {
        x.to_le()
    }

    pub fn to_little64(x: u64) -> u64 {
        x.to_le()
    }

    pub fn to_big16(x: u16) -> u16 {
        x.to_be()
    }

    pub fn to_big32(x: u32) -> u32 {
        x.to_be()
    }

    pub fn to_big64(x: u64) -> u64 {
        x.to_be()
    }
}

pub trait EndianConvert {
    fn swap_bytes(self) -> Self;
    fn to_little_endian(self) -> Self;
    fn to_big_endian(self) -> Self;
}

macro_rules! impl_endian_convert {
    ($($t:ty),*) => {
        $(
            impl EndianConvert for $t {
                fn swap_bytes(self) -> Self { <$t>::swap_bytes(self) }
                fn to_little_endian(self) -> Self { <$t>::to_le(self) }
                fn to_big_endian(self) -> Self { <$t>::to_be(self) }
            }
        )*
    };
}

impl_endian_convert!(u16, u32, u64);

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_swap16() {
        assert_eq!(Endian::swap16(0x1234), 0x3412);
        assert_eq!(EndianConvert::swap_bytes(0x1234u16), 0x3412);
    }

    #[test]
    fn test_swap32() {
        assert_eq!(Endian::swap32(0x12345678), 0x78563412);
        assert_eq!(EndianConvert::swap_bytes(0x12345678u32), 0x78563412);
    }

    #[test]
    fn test_swap64() {
        assert_eq!(
            Endian::swap64(0x123456789abcdef0),
            0xf0debc9a78563412
        );
        assert_eq!(
            EndianConvert::swap_bytes(0x123456789abcdef0u64),
            0xf0debc9a78563412
        );
    }

    #[test]
    fn test_endian_conversions() {
        let value = 0x1234u16;

        #[cfg(target_endian = "little")]
        {
            assert_eq!(EndianConvert::to_little_endian(value), value);
            assert_eq!(EndianConvert::to_big_endian(value), value.swap_bytes());
        }

        #[cfg(target_endian = "big")]
        {
            assert_eq!(EndianConvert::to_big_endian(value), value);
            assert_eq!(EndianConvert::to_little_endian(value), value.swap_bytes());
        }
    }
}
