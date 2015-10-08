#include "Ripemd.h"

using namespace Crypto;

Ripemd::Hash Ripemd::operator ()(std::vector<uint8_t> const & input)
{
    return operator()(&input[0], input.size());
}

Ripemd::Hash Ripemd::operator ()(uint8_t const * input, size_t length)
{
    return Hash();
}
