#include "Base58.h"

//#include <openssl/bn.h>
#include <algorithm>
#include <array>

static char const ENCODE_MAP[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
static std::array<int, 122 - 49 + 1> const DECODE_MAP =
{
    0, 1, 2, 3, 4, 5, 6, 7, 8,                              // '1' - '9' (49 - 57)
    -1, -1, -1, -1, -1, -1, -1,                             // ':' - '@' (58 - 64)
    9, 10, 11, 12, 13, 14, 15, 16,                          // 'A' - 'H' (65 - 72)
    -1,                                                     // 'I'       (73)
    17, 18, 19, 20, 21,                                     // 'J' - 'N' (74 - 78)
    -1,                                                     // 'O'       (79)
    22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,             // 'P' - 'Z' (80 - 90)
    -1, -1, -1, -1, -1, -1,                                 // '[' - '`' (91 - 96)
    33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,             // 'a' - 'k' (97 - 107)
    -1,                                                     // 'l'       (108)
    44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57  // 'm' - 'z' (109 - 122)
};

static char encode(int x)
{
    return ENCODE_MAP[x];
}

static int decode(char c)
{
    c -= '1';    // first valid char is '1'

    // Valid range is '1' to 'z'
    if (c < 0 || c >= DECODE_MAP.size())
        return -1;

    return DECODE_MAP[c];
}
std::string Base58::operator()(std::vector<uint8_t> const & input)
{
    return operator()(&input[0], input.size());
}

std::string Base58::operator()(uint8_t const * input, size_t length)
{

    BIGNUM *i = BN_new();
    if (!i)
        return "";

    BN_bin2bn(input, length, i);  // Note: input is considered to be big-endian

    std::string output;

    do
    {
        BN_ULONG r = BN_div_word(i, 58);
        output.push_back(encode(r));
    } while (!BN_is_zero(i));

    std::reverse(output.begin(), output.end());

    return output;
}

bool decode(std::string const & input, std::vector<uint8_t> & output)
{
    return decode(input.c_str(), input.length(), output);
}

bool decode(char const * input, size_t length, std::vector<uint8_t> & output)
{

}
