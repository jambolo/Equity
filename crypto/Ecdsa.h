#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{

    class Ecdsa
    {
    public:

        typedef std::vector<uint8_t> PublicKey;
        typedef std::vector<uint8_t> PrivateKey;
    };

}