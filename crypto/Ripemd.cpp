#include "Ripemd.h"

#include <algorithm>
#include <cassert>

namespace Crypto
{

Ripemd160Hash ripemd160(std::vector<uint8_t> const & input)
{
    return ripemd160(input.data(), input.size());
}

Ripemd160Hash ripemd160(uint8_t const * input, size_t length)
{
    Ripemd160Hash hash = {};
    return hash;
}

} // namespace Crypto
