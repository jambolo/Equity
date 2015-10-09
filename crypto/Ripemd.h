#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{
        size_t const RIPEMD160_HASH_SIZE = 160 / 8;
        typedef std::vector<uint8_t> Ripemd160Hash;

        Ripemd160Hash ripemd160(std::vector<uint8_t> const & input);
        Ripemd160Hash ripemd160(uint8_t const * input, size_t length);
}