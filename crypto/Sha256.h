#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{
        size_t const SHA256_HASH_SIZE = 256 / 8;
        typedef std::vector<uint8_t> Sha256Hash;

        Sha256Hash sha256(std::vector<uint8_t> const & input);
        Sha256Hash sha256(uint8_t const * input, size_t length);
}