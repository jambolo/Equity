//! `extern "C"` shims around the Rust hash crates so the C++ side (crypto/Sha1.cpp,
//! Sha512.cpp, Ripemd.cpp, Pbkdf2.cpp — still compiled into equity_bridge for
//! ScriptEngine.cpp + Mnemonic.cpp callers) can delegate to Rust implementations.
//! Symbols are pinned via the `#[used]` table below; equity's lib.rs adds an
//! additional keepalive that reaches across the rlib boundary.
//!
//! All exports are FFI plumbing called only from C++; safety contracts are
//! enforced by the C++ caller (valid pointer + size).

#![allow(clippy::missing_safety_doc, clippy::missing_transmute_annotations)]

use crate::{hmac, pbkdf2, ripemd, sha1, sha256, sha512};

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_sha256(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let hash = sha256::sha256(slice);
    unsafe { std::ptr::copy_nonoverlapping(hash.as_ptr(), out, sha256::SHA256_HASH_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_double_sha256(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let hash = sha256::double_sha256(slice);
    unsafe { std::ptr::copy_nonoverlapping(hash.as_ptr(), out, sha256::SHA256_HASH_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_checksum(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let chk = sha256::checksum(slice);
    unsafe { std::ptr::copy_nonoverlapping(chk.as_ptr(), out, sha256::CHECKSUM_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_sha1(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let hash = sha1::sha1(slice);
    unsafe { std::ptr::copy_nonoverlapping(hash.as_ptr(), out, sha1::SHA1_HASH_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_sha512(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let hash = sha512::sha512(slice);
    unsafe { std::ptr::copy_nonoverlapping(hash.as_ptr(), out, sha512::SHA512_HASH_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ripemd160(input: *const u8, length: usize, out: *mut u8) {
    let slice = if input.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(input, length) }
    };
    let hash = ripemd::ripemd160(slice);
    unsafe { std::ptr::copy_nonoverlapping(hash.as_ptr(), out, ripemd::RIPEMD160_HASH_SIZE) };
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_hmac_sha512(
    key: *const u8,
    key_size: usize,
    message: *const u8,
    message_size: usize,
    out: *mut u8,
) {
    let k = if key.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(key, key_size) }
    };
    let m = if message.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(message, message_size) }
    };
    let mac = hmac::hmac_sha512(k, m);
    unsafe { std::ptr::copy_nonoverlapping(mac.as_ptr(), out, hmac::HMAC_SHA512_SIZE) };
}

/// Returns true on success; fills `out_capacity` bytes into `out`.
#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_pbkdf2_hmac_sha512(
    password: *const u8,
    password_size: usize,
    salt: *const u8,
    salt_size: usize,
    count: i32,
    out: *mut u8,
    out_capacity: usize,
) -> bool {
    let p = if password.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(password, password_size) }
    };
    let s = if salt.is_null() {
        &[][..]
    } else {
        unsafe { std::slice::from_raw_parts(salt, salt_size) }
    };
    match pbkdf2::pbkdf2_hmac_sha512(p, s, count, out_capacity) {
        Ok(v) => {
            unsafe { std::ptr::copy_nonoverlapping(v.as_ptr(), out, v.len()) };
            true
        }
        Err(_) => false,
    }
}

type HashFn = unsafe extern "C" fn();

#[used]
static HASH_FFI_KEEPALIVE: [HashFn; 8] = [
    // SAFETY: erased signatures; entries exist only to anchor `#[used]` so the
    // linker preserves the extern "C" exports across the rlib boundary.
    unsafe { std::mem::transmute(crypto_sha1 as *const ()) },
    unsafe { std::mem::transmute(crypto_sha256 as *const ()) },
    unsafe { std::mem::transmute(crypto_double_sha256 as *const ()) },
    unsafe { std::mem::transmute(crypto_checksum as *const ()) },
    unsafe { std::mem::transmute(crypto_sha512 as *const ()) },
    unsafe { std::mem::transmute(crypto_ripemd160 as *const ()) },
    unsafe { std::mem::transmute(crypto_hmac_sha512 as *const ()) },
    unsafe { std::mem::transmute(crypto_pbkdf2_hmac_sha512 as *const ()) },
];
