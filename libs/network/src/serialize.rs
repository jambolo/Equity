//! Wire-format primitives for Bitcoin network messages.
//!
//! All integers are little-endian. Variable-length integers (CompactSize) and
//! variable-length strings follow the Bitcoin protocol spec.

use anyhow::{Result, anyhow, bail};

/// Append `v` to `out`.
pub fn write_u8(out: &mut Vec<u8>, v: u8) {
    out.push(v);
}

/// Append `v` as little-endian 2 bytes.
pub fn write_u16(out: &mut Vec<u8>, v: u16) {
    out.extend_from_slice(&v.to_le_bytes());
}

/// Append `v` as little-endian 4 bytes.
pub fn write_u32(out: &mut Vec<u8>, v: u32) {
    out.extend_from_slice(&v.to_le_bytes());
}

/// Append `v` as little-endian 8 bytes.
pub fn write_u64(out: &mut Vec<u8>, v: u64) {
    out.extend_from_slice(&v.to_le_bytes());
}

/// Append raw bytes `v` to `out` without any length prefix.
pub fn write_bytes(out: &mut Vec<u8>, v: &[u8]) {
    out.extend_from_slice(v);
}

/// Append `v` encoded as a Bitcoin CompactSize varint (1, 3, 5, or 9 bytes).
pub fn write_var_int(out: &mut Vec<u8>, v: u64) {
    if v < 0xfd {
        out.push(v as u8);
    } else if v <= 0xffff {
        out.push(0xfd);
        write_u16(out, v as u16);
    } else if v <= 0xffff_ffff {
        out.push(0xfe);
        write_u32(out, v as u32);
    } else {
        out.push(0xff);
        write_u64(out, v);
    }
}

/// Append `s` as a CompactSize length prefix followed by raw UTF-8 bytes.
pub fn write_var_string(out: &mut Vec<u8>, s: &str) {
    write_var_int(out, s.len() as u64);
    out.extend_from_slice(s.as_bytes());
}

/// Append `v` as a CompactSize length prefix followed by the raw bytes.
pub fn write_var_bytes(out: &mut Vec<u8>, v: &[u8]) {
    write_var_int(out, v.len() as u64);
    out.extend_from_slice(v);
}

/// Read one byte from `stream` and advance it. Errors if empty.
pub fn read_u8(stream: &mut &[u8]) -> Result<u8> {
    if stream.is_empty() {
        bail!("truncated u8");
    }
    let v = stream[0];
    *stream = &stream[1..];
    Ok(v)
}

/// Read a little-endian `u16` and advance the cursor.
pub fn read_u16(stream: &mut &[u8]) -> Result<u16> {
    if stream.len() < 2 {
        bail!("truncated u16");
    }
    let (head, rest) = stream.split_at(2);
    *stream = rest;
    Ok(u16::from_le_bytes([head[0], head[1]]))
}

/// Read a little-endian `u32` and advance the cursor.
pub fn read_u32(stream: &mut &[u8]) -> Result<u32> {
    if stream.len() < 4 {
        bail!("truncated u32");
    }
    let (head, rest) = stream.split_at(4);
    *stream = rest;
    Ok(u32::from_le_bytes([head[0], head[1], head[2], head[3]]))
}

/// Read a little-endian `u64` and advance the cursor.
pub fn read_u64(stream: &mut &[u8]) -> Result<u64> {
    if stream.len() < 8 {
        bail!("truncated u64");
    }
    let (head, rest) = stream.split_at(8);
    *stream = rest;
    Ok(u64::from_le_bytes([
        head[0], head[1], head[2], head[3], head[4], head[5], head[6], head[7],
    ]))
}

/// Borrow `n` bytes from `stream` and advance the cursor. Errors on short input.
pub fn read_bytes<'a>(stream: &mut &'a [u8], n: usize) -> Result<&'a [u8]> {
    if stream.len() < n {
        bail!(
            "truncated bytes ({} requested, {} available)",
            n,
            stream.len()
        );
    }
    let (head, rest) = stream.split_at(n);
    *stream = rest;
    Ok(head)
}

/// Read a fixed-size `[u8; N]` and advance the cursor. Errors on short input.
pub fn read_array<const N: usize>(stream: &mut &[u8]) -> Result<[u8; N]> {
    let head = read_bytes(stream, N)?;
    let mut out = [0u8; N];
    out.copy_from_slice(head);
    Ok(out)
}

/// Read a Bitcoin CompactSize varint and advance the cursor.
pub fn read_var_int(stream: &mut &[u8]) -> Result<u64> {
    let tag = read_u8(stream)?;
    match tag {
        0xff => read_u64(stream),
        0xfe => Ok(read_u32(stream)? as u64),
        0xfd => Ok(read_u16(stream)? as u64),
        v => Ok(v as u64),
    }
}

/// Read a CompactSize-prefixed UTF-8 string and advance the cursor.
pub fn read_var_string(stream: &mut &[u8]) -> Result<String> {
    let n = read_var_int(stream)? as usize;
    let bytes = read_bytes(stream, n)?;
    String::from_utf8(bytes.to_vec()).map_err(|e| anyhow!("invalid utf-8 var string: {}", e))
}

/// Read a CompactSize-prefixed byte string and advance the cursor.
pub fn read_var_bytes(stream: &mut &[u8]) -> Result<Vec<u8>> {
    let n = read_var_int(stream)? as usize;
    let bytes = read_bytes(stream, n)?;
    Ok(bytes.to_vec())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn var_int_round_trip() {
        for v in [
            0u64,
            0xfc,
            0xfd,
            0xffff,
            0x10000,
            0xffff_ffff,
            0x1_0000_0000,
            u64::MAX,
        ] {
            let mut out = Vec::new();
            write_var_int(&mut out, v);
            let mut s = &out[..];
            assert_eq!(read_var_int(&mut s).unwrap(), v);
            assert!(s.is_empty());
        }
    }

    #[test]
    fn var_string_round_trip() {
        let mut out = Vec::new();
        write_var_string(&mut out, "hello/equity");
        let mut s = &out[..];
        assert_eq!(read_var_string(&mut s).unwrap(), "hello/equity");
    }

    #[test]
    fn integer_endianness_is_little() {
        let mut out = Vec::new();
        write_u32(&mut out, 0x12345678);
        assert_eq!(out, vec![0x78, 0x56, 0x34, 0x12]);
    }

    #[test]
    fn truncation_errors() {
        let mut s: &[u8] = &[];
        assert!(read_u32(&mut s).is_err());
        let mut s: &[u8] = &[0xfd, 0x01];
        assert!(read_var_int(&mut s).is_err());
    }

    #[test]
    fn var_bytes_round_trip() {
        let mut out = Vec::new();
        write_var_bytes(&mut out, &[0xAA, 0xBB, 0xCC, 0xDD]);
        let mut s = &out[..];
        assert_eq!(
            read_var_bytes(&mut s).unwrap(),
            vec![0xAA, 0xBB, 0xCC, 0xDD]
        );
        assert!(s.is_empty());
    }

    #[test]
    fn invalid_utf8_var_string_rejected() {
        let mut out = Vec::new();
        write_var_int(&mut out, 2);
        out.extend_from_slice(&[0xFF, 0xFE]);
        let mut s = &out[..];
        assert!(read_var_string(&mut s).is_err());
    }

    #[test]
    fn read_u8_round_trip() {
        let mut out = Vec::new();
        write_u8(&mut out, 0x42);
        let mut s = &out[..];
        assert_eq!(read_u8(&mut s).unwrap(), 0x42);
    }

    #[test]
    fn read_array_truncated() {
        let mut s: &[u8] = &[1, 2, 3];
        assert!(read_array::<4>(&mut s).is_err());
    }

    proptest::proptest! {
        #[test]
        fn prop_u32_round_trip(v: u32) {
            let mut out = Vec::new();
            write_u32(&mut out, v);
            let mut s = &out[..];
            proptest::prop_assert_eq!(read_u32(&mut s).unwrap(), v);
        }

        #[test]
        fn prop_u64_round_trip(v: u64) {
            let mut out = Vec::new();
            write_u64(&mut out, v);
            let mut s = &out[..];
            proptest::prop_assert_eq!(read_u64(&mut s).unwrap(), v);
        }

        #[test]
        fn prop_var_int_round_trip(v: u64) {
            let mut out = Vec::new();
            write_var_int(&mut out, v);
            let mut s = &out[..];
            proptest::prop_assert_eq!(read_var_int(&mut s).unwrap(), v);
        }

        #[test]
        fn prop_var_bytes_round_trip(data in proptest::collection::vec(any::<u8>(), 0..512)) {
            let mut out = Vec::new();
            write_var_bytes(&mut out, &data);
            let mut s = &out[..];
            proptest::prop_assert_eq!(read_var_bytes(&mut s).unwrap(), data);
        }
    }

    use proptest::prelude::any;
}
