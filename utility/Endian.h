#pragma once

#include <cstdint>

#define TARGET_LITTLE_ENDIAN

namespace Utility
{

namespace Endian
{

inline uint16_t swap(uint16_t x)
{
    uint8_t high = (uint8_t)(x & 0xff);
    uint8_t low  = (uint8_t)((x >> 8) & 0xff);
    return ((uint16_t)high << 8) | (uint16_t)low;
}

inline uint32_t swap(uint32_t x)
{
    uint16_t high = swap((uint16_t)(x & 0xffff));
    uint16_t low  = swap((uint16_t)((x >> 16) & 0xffff));
    return ((uint32_t)high << 16) | (uint32_t)low;
}

inline uint64_t swap(uint64_t x)
{
    uint32_t high = swap((uint32_t)(x & 0xffffffff));
    uint32_t low  = swap((uint32_t)((x >> 32) & 0xffffffff));
    return ((uint64_t)high << 32) | (uint64_t)low;
}

// Note: Conversions in each direction are identical, so the same function is used for both.

#if defined(TARGET_LITTLE_ENDIAN)

inline uint8_t  little(uint8_t x) { return x; }
inline uint16_t little(uint16_t x) { return x; }
inline uint32_t little(uint32_t x) { return x; }
inline uint64_t little(uint64_t x) { return x; }

inline uint8_t  big(uint8_t x) { return x; }
inline uint16_t big(uint16_t x) { return swap(x); }
inline uint32_t big(uint32_t x) { return swap(x); }
inline uint64_t big(uint64_t x) { return swap(x); }

#elif defined(TARGET_BIG_ENDIAN)

inline uint8_t  little(uint8_t x) { return x; }
inline uint16_t little(uint16_t x) { return swap(x); }
inline uint32_t little(uint32_t x) { return swap(x); }
inline uint64_t little(uint64_t x) { return swap(x); }

inline uint8_t  big(uint8_t x) { return x; }
inline uint16_t big(uint16_t x) { return x; }
inline uint32_t big(uint32_t x) { return x; }
inline uint64_t big(uint64_t x) { return x; }

#else // if defined(TARGET_LITTLE_ENDIAN)
  #error TARGET_LITTLE_ENDIAN or TARGET_LITTLE_ENDIAN (but not both) must be defined
#endif // if defined(TARGET_LITTLE_ENDIAN)

} // namespace Endian
} // namespace Utility
