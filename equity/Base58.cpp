#include "Base58.h"

#include "utility/Debug.h"

#include <wolfssl/options.h>
#include <wolfssl/wolfcrypt/settings.h>
#include <wolfssl/wolfcrypt/sp_int.h>

#include <algorithm>
#include <array>
#include <memory>

using namespace Equity;

static char const ENCODE_MAP[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static int const DECODE_MAP[]  =
{
    0,  1,  2,  3,  4,  5,  6,  7,  8,                     // '1' - '9' (49 - 57)
    -1, -1, -1, -1, -1, -1, -1,                            // ':' - '@' (58 - 64)
    9,  10, 11, 12, 13, 14, 15, 16,                        // 'A' - 'H' (65 - 72)
    -1,                                                    // 'I'       (73)
    17, 18, 19, 20, 21,                                    // 'J' - 'N' (74 - 78)
    -1,                                                    // 'O'       (79)
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,            // 'P' - 'Z' (80 - 90)
    -1, -1, -1, -1, -1, -1,                                // '[' - '`' (91 - 96)
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,            // 'a' - 'k' (97 - 107)
    -1,                                                    // 'l'       (108)
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57 // 'm' - 'z' (109 - 122)
};
static size_t const DECODE_MAP_SIZE = sizeof(DECODE_MAP) / sizeof(DECODE_MAP[0]);

static char encode(int x)
{
    return ENCODE_MAP[x];
}

static int decode(int c)
{
    c -= '1';    // first valid char is '1'

    // Valid range is '1' to 'z'
    if (c < 0 || c >= DECODE_MAP_SIZE)
    {
        return -1;
    }

    return DECODE_MAP[c];
}

std::string Base58::encode(std::vector<uint8_t> const & input)
{
    return encode(input.data(), input.size());
}

std::string Base58::encode(uint8_t const * input, size_t length)
{
    NOT_YET_IMPLEMENTED();

    std::string output;
    return output;
}

bool Base58::decode(std::string const & input, std::vector<uint8_t> & output)
{
    return decode(input.c_str(), output);
}

bool Base58::decode(char const * input, std::vector<uint8_t> & output)
{
    if (*input == 0)
    {
        return false;
    }

    NOT_YET_IMPLEMENTED();
    return false;
}
