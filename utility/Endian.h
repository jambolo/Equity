#pragma once

#include <cstdint>

#define TARGET_LITTLE_ENDIAN

namespace Utility
{

    inline uint16_t swapEndian(uint16_t x)
    {
        uint8_t high = (uint8_t)(       x & 0xff);
        uint8_t low  = (uint8_t)((x >> 8) & 0xff);
        return ((uint16_t)high << 8) | (uint16_t)low;
    }

    inline uint32_t swapEndian(uint32_t x)
    {
        uint16_t high = swapEndian((uint16_t)(        x & 0xffff));
        uint16_t low  = swapEndian((uint16_t)((x >> 16) & 0xffff));
        return ((uint32_t)high << 16) | (uint32_t)low;
    }

    inline uint64_t swapEndian(uint64_t x)
    {
        uint32_t high = swapEndian((uint32_t)(        x & 0xffffffff));
        uint32_t low  = swapEndian((uint32_t)((x >> 32) & 0xffffffff));
        return ((uint64_t)high << 32) | (uint64_t)low;
    }

#if defined(TARGET_LITTLE_ENDIAN)

    inline uint16_t littleEndian(uint16_t x) { return x; }
    inline uint32_t littleEndian(uint32_t x) { return x; }
    inline uint64_t littleEndian(uint64_t x) { return x; }

    inline uint16_t bigEndian(uint16_t x) { return swapEndian(x); }
    inline uint32_t bigEndian(uint32_t x) { return swapEndian(x); }
    inline uint64_t bigEndian(uint64_t x) { return swapEndian(x); }

#elif defined(TARGET_BIG_ENDIAN)

    inline uint16_t littleEndian(uint16_t x) { return swapEndian(x); }
    inline uint32_t littleEndian(uint32_t x) { return swapEndian(x); }
    inline uint64_t littleEndian(uint64_t x) { return swapEndian(x); }

    inline uint16_t bigEndian(uint16_t x) { return x; }
    inline uint32_t bigEndian(uint32_t x) { return x; }
    inline uint64_t bigEndian(uint64_t x) { return x; }

#else
#error TARGET_LITTLE_ENDIAN or TARGET_LITTLE_ENDIAN (but not both) must be defined
#endif

} // namespace Utility