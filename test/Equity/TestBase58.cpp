#include "equity/Base58.h"
#include "utility/Utility.h"

#include <string>
#include <vector>

using namespace Equity;

int TestBase58()
{
    int errors = 0;

    struct TestInput
    {
        size_t size;
        uint8_t data[256];
        char const * stringForm;
    };
    static TestInput const ENCODE_CASES[] =
    {
        { 1, { 0 }, "1" },
        { 1, { 0xff }, "5Q" },
        { 20, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1" },
        { 20, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, "4ZrjxJnU1LA5xSyrWMNuXTvSYKwt" },
        { 20, { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE }, "qb3y62fmEEVTPySXPQ77WXok6H" },
        { 32, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1" },
        { 32, { 0xA1, 0xD8, 0x50, 0x84, 0x5A, 0x07, 0x76, 0xE0, 0xC8, 0x59, 0x64, 0x4A, 0x67, 0x3F, 0xAF, 0x7A, 0x55, 0x2E, 0x0B, 0x76, 0xEE, 0xFF, 0xAA, 0x91, 0x3E, 0xEF, 0xB7, 0x7E, 0x55, 0xE8, 0x19, 0x6A }, "Btmx3Rk8vxJPaGy2XcwHfoH5gUd88cVSNUog2VgejZrH" },
        { 32, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40 }, "JEKNVnkbo3jma5nREBBJCD7MJVUPAg5THBwPPejEsG9u" },
        { 43, { 0x00, 0x01, 0x11, 0xD3, 0x8E, 0x5F, 0xC9, 0x07, 0x1F, 0xFC, 0xD2, 0x0B, 0x4A, 0x76, 0x3C, 0xC9, 0xAE, 0x4F, 0x25, 0x2B, 0xB4, 0xE4, 0x8F, 0xD6, 0x6A, 0x83, 0x5E, 0x25, 0x2A, 0xDA, 0x93, 0xFF, 0x48, 0x0D, 0x6D, 0xD4, 0x3D, 0xC6, 0x2A, 0x64, 0x11, 0x55, 0xA5 }, "23456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz" }
    };

    static TestInput const DECODE_CASES[] =
    {
        { 1, { 0 }, "1" },
        { 1, { 0xff }, "5Q" },
        { 20, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "11111111111111111111" },
        { 20, { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff }, "4ZrjxJnU1LA5xSyrWMNuXTvSYKwt" },
        { 20, { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE }, "qb3y62fmEEVTPySXPQ77WXok6H" },
        { 32, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "11111111111111111111111111111111" },
        { 32, { 0xA1, 0xD8, 0x50, 0x84, 0x5A, 0x07, 0x76, 0xE0, 0xC8, 0x59, 0x64, 0x4A, 0x67, 0x3F, 0xAF, 0x7A, 0x55, 0x2E, 0x0B, 0x76, 0xEE, 0xFF, 0xAA, 0x91, 0x3E, 0xEF, 0xB7, 0x7E, 0x55, 0xE8, 0x19, 0x6A }, "Btmx3Rk8vxJPaGy2XcwHfoH5gUd88cVSNUog2VgejZrH" },
        { 32, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40 }, "JEKNVnkbo3jma5nREBBJCD7MJVUPAg5THBwPPejEsG9u" },
        { 43, { 0x00, 0x01, 0x11, 0xD3, 0x8E, 0x5F, 0xC9, 0x07, 0x1F, 0xFC, 0xD2, 0x0B, 0x4A, 0x76, 0x3C, 0xC9, 0xAE, 0x4F, 0x25, 0x2B, 0xB4, 0xE4, 0x8F, 0xD6, 0x6A, 0x83, 0x5E, 0x25, 0x2A, 0xDA, 0x93, 0xFF, 0x48, 0x0D, 0x6D, 0xD4, 0x3D, 0xC6, 0x2A, 0x64, 0x11, 0x55, 0xA5 }, "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz" }
    };

    struct DecodeValidityTestInput
    {
        char const * data;
        bool expected;
    };
    static DecodeValidityTestInput const DECODE_VALIDITY_CASES[] =
    {
        { "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz", true },
        { "", false },
        { "0", false },
        { "I", false },
        { "O", false },
        { "l", false },
    };

    printf("+-- Base58\n");

    printf("    +-- testing: std::string encode(uint8_t const * input, size_t length)\n");
    for (auto const & c : ENCODE_CASES)
    {
        std::string name = Utility::shorten(c.stringForm);
        std::string result = Base58::encode(c.data, c.size);
        if (result != c.stringForm)
        {
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), c.stringForm, result.c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: bool decode(char const * input, std::vector<uint8_t> & output)\n");
    for (auto const & c : DECODE_VALIDITY_CASES)
    {
        std::string name = Utility::shorten(c.data);
        std::vector<uint8_t> resultVector;

        bool ok = Base58::decode(c.data, resultVector);
        if (ok != c.expected)
        {
            printf("        +== %s: returned %s, expected %s\n", name.c_str(), ok ? "true" : "false", c.expected ? "true" : "false");
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }
    
    for (auto const & c : DECODE_CASES)
    {
        std::string name = Utility::shorten(Utility::toHex(c.data, c.size));
        std::vector<uint8_t> resultVector;

        bool ok = Base58::decode(c.stringForm, resultVector);
        if (!ok)
        {
            printf("        +== %s: failed\n", name.c_str());
            ++errors;
        }
        else if (resultVector.size() != c.size)
        {
            printf("        +== %s: expected size = %u, got size = %u\n", name.c_str(), (unsigned)c.size, (unsigned)resultVector.size());
            ++errors;
        }
        else if (!std::equal(resultVector.begin(), resultVector.end(), c.data))
        {
            printf("        +== %s: expected %s, got %s\n", name.c_str(), Utility::toHex(c.data, c.size).c_str(), Utility::toHex(resultVector).c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    return errors;
}
