#include "Random.h"

#include "utility/Debug.h"

#include <cassert>
#include <limits>

namespace Crypto
{
namespace Random
{

bool status()
{
    NOT_YET_IMPLEMENTED();
	return false;
}

void getBytes(uint8_t * buffer, size_t size)
{
    assert(size < std::numeric_limits<int>().max());
    NOT_YET_IMPLEMENTED();
}

void addEntropy(uint8_t const * buffer, size_t size, double entropy)
{
    assert(size < std::numeric_limits<int>().max());
    NOT_YET_IMPLEMENTED();
}

}  // namespace Random
}  // namespace Crypto
