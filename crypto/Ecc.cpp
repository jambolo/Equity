// Thin shim over the Rust `secp256k1` implementation exported by libs/crypto
// (see libs/crypto/src/ecc_ffi.rs). The pre-existing Ecc.h API is preserved so
// equity C++ code (PublicKey.cpp) keeps compiling without changes.

#include "Ecc.h"

#include <cstddef>
#include <cstdint>
#include <cstring>

extern "C" {

bool crypto_ecc_public_key_is_valid(uint8_t const * k, size_t size);
bool crypto_ecc_private_key_is_valid(uint8_t const * k, size_t size);
bool crypto_ecc_derive_public_key(uint8_t const * private_key,
                                  uint8_t *       out,
                                  size_t          out_capacity,
                                  size_t *        out_len,
                                  bool            uncompressed);
bool crypto_ecc_sign(uint8_t const * message,
                     size_t          message_size,
                     uint8_t const * private_key,
                     uint8_t *       out,
                     size_t          out_capacity,
                     size_t *        out_len);
bool crypto_ecc_verify(uint8_t const * message,
                       size_t          message_size,
                       uint8_t const * public_key,
                       size_t          public_key_size,
                       uint8_t const * signature,
                       size_t          signature_size);

} // extern "C"

bool Crypto::Ecc::publicKeyIsValid(uint8_t const * k, size_t size)
{
    return crypto_ecc_public_key_is_valid(k, size);
}

bool Crypto::Ecc::privateKeyIsValid(uint8_t const * k, size_t size)
{
    return crypto_ecc_private_key_is_valid(k, size);
}

bool Crypto::Ecc::derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey, bool uncompressed)
{
    uint8_t buf[Crypto::Ecc::UNCOMPRESSED_PUBLIC_KEY_SIZE];
    size_t  out_len = 0;
    if (!crypto_ecc_derive_public_key(prvKey.data(), buf, sizeof(buf), &out_len, uncompressed))
    {
        return false;
    }
    pubKey.assign(buf, buf + out_len);
    return true;
}

bool Crypto::Ecc::sign(uint8_t const * message, size_t size, PrivateKey const & prvKey, Signature & signature)
{
    uint8_t buf[72]; // max DER ECDSA signature
    size_t  out_len = 0;
    if (!crypto_ecc_sign(message, size, prvKey.data(), buf, sizeof(buf), &out_len))
    {
        return false;
    }
    signature.assign(buf, buf + out_len);
    return true;
}

bool Crypto::Ecc::verify(uint8_t const * message, size_t size, PublicKey const & pubKey, Signature const & signature)
{
    return crypto_ecc_verify(message, size, pubKey.data(), pubKey.size(), signature.data(), signature.size());
}
