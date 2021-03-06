#pragma once

#include <array>
#include <cstdint>
#include <vector>

//! Crypto library.
//!
//! The primary purpose of this library is to provide a generic interface wrapping a specific implementation.

namespace Crypto
{

//! ECC library.
//!
//! @note   Key values are assumed to be big-endian.

namespace Ecc
{

size_t const COMPRESSED_PUBLIC_KEY_SIZE     = 33;               //!< Size of a compressed public key
size_t const UNCOMPRESSED_PUBLIC_KEY_SIZE   = 65;               //!< Size of an uncompressed public key
size_t const PRIVATE_KEY_SIZE               = 256 / 8;          //!< Size of a private key

typedef std::vector<uint8_t> PublicKey;                         //!< An ECC public key
typedef std::array<uint8_t, PRIVATE_KEY_SIZE> PrivateKey;       //!< An ECC private key
typedef std::vector<uint8_t> Signature;                         //!< An ECC signature

//! Returns true if the public key is valid.
//!
//! @param  k       key
//! @param  size    size of key
//! @return true if the key is valid
bool publicKeyIsValid(uint8_t const * k, size_t size);

//! Returns true if the public key is valid.
//!
//! @param  k       key
//! @return true if the key is valid
bool publicKeyIsValid(PublicKey const & k);

//! Returns true if the private key is valid.
//!
//! @param  k       key
//! @param  size    size of key
//! @return true if the key is valid
bool privateKeyIsValid(uint8_t const * k, size_t size);

//! Returns true if the private key is valid.
//!
//! @param  k       key
//! @return true if the key is valid
bool privateKeyIsValid(PrivateKey const & k);

//! Derives a public key from a private key.
//!
//! @param      prvKey          private key
//! @param[out] pubKey          derived public key
//! @param      uncompressed    if true, then the resulting public key is uncompressed (default: false)
//! @return true if the returned key is valid
bool derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey, bool uncompressed = false);

//! Signs a message.
//!
//! @param      message     message to sign
//! @param      size        size of the message
//! @param      prvKey      private key
//! @param      pubKey      public key
//! @param[out] signature   signature
//! @return true if the returned signature is valid
bool sign(uint8_t const * message, size_t size, PrivateKey const & prvKey, PublicKey const & pubKey, Signature & signature);

//! Verifies a signed message.
//!
//! @param      message     message to sign
//! @param      size        size of the message
//! @param      pubKey      public key
//! @param      signature   signature
//! @return true if the message's signature is valid and it matches the message
bool verify(uint8_t const * message, size_t size, PublicKey const & pubKey, Signature const & signature);

/********************************************************************************************************************/

inline bool publicKeyIsValid(uint8_t const * k, size_t size)
{
    return (size == UNCOMPRESSED_PUBLIC_KEY_SIZE && k[0] == 4) ||
           (size == COMPRESSED_PUBLIC_KEY_SIZE && (k[0] == 2 || k[0] == 3));
}

inline bool publicKeyIsValid(PublicKey const & k)
{
    return publicKeyIsValid(k.data(), k.size());
}

inline bool privateKeyIsValid(PrivateKey const & k)
{
    return privateKeyIsValid(k.data(), k.size());
}

} // namespace Ecc
} // namespace Crypto
