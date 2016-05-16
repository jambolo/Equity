#pragma once

#include <array>
#include <cstdint>

namespace Crypto
{

//! ECDSA library.
//!
//! @note   Key values are assumed to be big-endian.
//! @todo   Move all ECDSA operations to here

class Ecdsa
{
public:

    static size_t const PUBLIC_KEY_SIZE = 256 / 8 + 1;          //!< Size of an uncompressed public key
    static size_t const PRIVATE_KEY_SIZE = 256 / 8;             //!< Size of a private key
    typedef std::array<uint8_t, PUBLIC_KEY_SIZE> PublicKey;     //!< An ECDSA public key
    typedef std::array<uint8_t, PRIVATE_KEY_SIZE> PrivateKey;   //!< An ECDSA private key

    //! Derives a public key from a private key
    bool derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey);

};

} // namespace Crypto
