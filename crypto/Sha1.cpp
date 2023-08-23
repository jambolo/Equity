#include "Sha1.h"

#include <algorithm>
#include <cassert>

namespace Crypto
{

Sha1Hash sha1(std::vector<uint8_t> const & input)
{
    return sha1(input.data(), input.size());
}

Sha1Hash sha1(uint8_t const * input, size_t length)
{
    Sha1Hash hash = {};
    return hash;
}

} // namespace Crypto
