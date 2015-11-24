#pragma once

#include <cstdint>
#include <vector>

namespace Crypto
{

// IMPORTANT NOTE: Key values are always big-endian.
class Ecdsa
{
public:

    typedef std::vector<uint8_t> PublicKey;
    typedef std::vector<uint8_t> PrivateKey;
};

} // namespace Crypto
