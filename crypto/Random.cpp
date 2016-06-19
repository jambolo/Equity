#include "Random.h"

#include "openssl/rand.h"

#include <cassert>
#include <limits>

namespace Crypto
{
namespace Random
{

void getBytes(uint8_t * buffer, size_t size)
{
    assert(size < std::numeric_limits<int>().max());
    RAND_bytes(buffer, (int)size);
}

void addEntropy(uint8_t const * buffer, size_t size, double entropy)
{
    assert(size < std::numeric_limits<int>().max());
    RAND_add(buffer, (int)size, entropy);
}

}  // namespace Random
}  // namespace Crypto
