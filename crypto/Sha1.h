#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{
    size_t const SHA1_HASH_SIZE = 160 / 8;
    typedef std::vector<uint8_t> Sha1Hash;

    Sha1Hash sha1(std::vector<uint8_t> const & input);
    Sha1Hash sha1(uint8_t const * input, size_t length);
}