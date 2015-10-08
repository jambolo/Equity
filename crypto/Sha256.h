#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{

    class Sha256
    {
    public:

        static size_t const SIZE = 256 / 8;
        typedef std::vector<uint8_t> Hash;

        Hash operator ()(std::vector<uint8_t> const & input);
        Hash operator ()(uint8_t const * input, size_t length);
    };

}