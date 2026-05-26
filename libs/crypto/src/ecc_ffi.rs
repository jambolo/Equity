//! `extern "C"` shims around `crate::ecc` so the C++ side (e.g. crypto/Ecc.cpp,
//! still compiled into the equity bridge for PublicKey.cpp/PrivateKey.cpp) can
//! call into the Rust `secp256k1` implementation. Symbols are kept live via a
//! `#[used]` table below.
//!
//! All exports are FFI plumbing called only from C++; safety contracts are
//! enforced by the C++ caller (valid pointer + size).

#![allow(clippy::missing_safety_doc, clippy::missing_transmute_annotations)]

use crate::ecc;

/// `out` must point to a buffer of at least 65 bytes. On success, `*out_len`
/// receives the actual length (33 compressed, 65 uncompressed).
#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ecc_public_key_is_valid(k: *const u8, size: usize) -> bool {
    if k.is_null() {
        return false;
    }
    let slice = unsafe { std::slice::from_raw_parts(k, size) };
    ecc::public_key_is_valid(slice)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ecc_private_key_is_valid(k: *const u8, size: usize) -> bool {
    if k.is_null() || size != ecc::PRIVATE_KEY_SIZE {
        return false;
    }
    let mut buf = [0u8; ecc::PRIVATE_KEY_SIZE];
    buf.copy_from_slice(unsafe { std::slice::from_raw_parts(k, size) });
    ecc::private_key_is_valid(&buf)
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ecc_derive_public_key(
    private_key: *const u8,
    out: *mut u8,
    out_capacity: usize,
    out_len: *mut usize,
    uncompressed: bool,
) -> bool {
    if private_key.is_null() || out.is_null() || out_len.is_null() {
        return false;
    }
    let mut sk = [0u8; ecc::PRIVATE_KEY_SIZE];
    sk.copy_from_slice(unsafe { std::slice::from_raw_parts(private_key, ecc::PRIVATE_KEY_SIZE) });
    match ecc::derive_public_key(&sk, uncompressed) {
        Ok(pk) => {
            if pk.len() > out_capacity {
                return false;
            }
            unsafe {
                std::ptr::copy_nonoverlapping(pk.as_ptr(), out, pk.len());
                *out_len = pk.len();
            }
            true
        }
        Err(_) => false,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ecc_sign(
    message: *const u8,
    message_size: usize,
    private_key: *const u8,
    out: *mut u8,
    out_capacity: usize,
    out_len: *mut usize,
) -> bool {
    if message.is_null() || private_key.is_null() || out.is_null() || out_len.is_null() {
        return false;
    }
    let msg = unsafe { std::slice::from_raw_parts(message, message_size) };
    let mut sk = [0u8; ecc::PRIVATE_KEY_SIZE];
    sk.copy_from_slice(unsafe { std::slice::from_raw_parts(private_key, ecc::PRIVATE_KEY_SIZE) });
    match ecc::sign(msg, &sk) {
        Ok(sig) => {
            if sig.len() > out_capacity {
                return false;
            }
            unsafe {
                std::ptr::copy_nonoverlapping(sig.as_ptr(), out, sig.len());
                *out_len = sig.len();
            }
            true
        }
        Err(_) => false,
    }
}

#[unsafe(no_mangle)]
pub unsafe extern "C" fn crypto_ecc_verify(
    message: *const u8,
    message_size: usize,
    public_key: *const u8,
    public_key_size: usize,
    signature: *const u8,
    signature_size: usize,
) -> bool {
    if message.is_null() || public_key.is_null() || signature.is_null() {
        return false;
    }
    let msg = unsafe { std::slice::from_raw_parts(message, message_size) };
    let pk = unsafe { std::slice::from_raw_parts(public_key, public_key_size) }.to_vec();
    let sig = unsafe { std::slice::from_raw_parts(signature, signature_size) }.to_vec();
    ecc::verify(msg, &pk, &sig)
}

type EccFn = unsafe extern "C" fn();

#[used]
static ECC_FFI_KEEPALIVE: [EccFn; 5] = [
    // SAFETY: erased signatures only used to anchor `#[used]` so the linker
    // preserves the extern "C" exports across the rlib boundary.
    unsafe { std::mem::transmute(crypto_ecc_public_key_is_valid as *const ()) },
    unsafe { std::mem::transmute(crypto_ecc_private_key_is_valid as *const ()) },
    unsafe { std::mem::transmute(crypto_ecc_derive_public_key as *const ()) },
    unsafe { std::mem::transmute(crypto_ecc_sign as *const ()) },
    unsafe { std::mem::transmute(crypto_ecc_verify as *const ()) },
];
