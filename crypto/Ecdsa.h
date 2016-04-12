#pragma once

#include <cstdint>
#include <array>

namespace Crypto
{

// IMPORTANT NOTE: Key values are assumed to be big-endian.
class Ecdsa
{
public:

    static size_t const PUBLIC_KEY_SIZE = 256 / 8 + 1;
    static size_t const PRIVATE_KEY_SIZE = 256 / 8;
    typedef std::array<uint8_t, PUBLIC_KEY_SIZE> PublicKey;
    typedef std::array<uint8_t, PRIVATE_KEY_SIZE> PrivateKey;

    bool derivePublicKey(PrivateKey const & prvKey, PublicKey & pubKey);
};

} // namespace Crypto
