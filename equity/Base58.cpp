#include "Base58.h"

#include <cstring>

using namespace Equity;

static char const ALPHABET[] =
    "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

static int decodeChar(char c)
{
    char const * p = std::strchr(ALPHABET, c);
    return p ? static_cast<int>(p - ALPHABET) : -1;
}

std::string Base58::encode(std::vector<uint8_t> const & input)
{
    return encode(input.data(), input.size());
}

std::string Base58::encode(uint8_t const * input, size_t length)
{
    // Leading zero bytes encode as leading '1' characters.
    size_t zeros = 0;
    while (zeros < length && input[zeros] == 0)
    {
        ++zeros;
    }

    // Worst-case base58 digit count: ceil(N * log(256) / log(58)) ~= N * 138 / 100 + 1.
    size_t const digits_capacity = (length - zeros) * 138 / 100 + 1;
    std::vector<uint8_t> digits(digits_capacity, 0);
    size_t digits_len = 0;

    for (size_t i = zeros; i < length; ++i)
    {
        int carry = input[i];
        size_t j = 0;
        for (auto it = digits.rbegin(); (carry != 0 || j < digits_len) && it != digits.rend(); ++it, ++j)
        {
            carry += 256 * (*it);
            *it = static_cast<uint8_t>(carry % 58);
            carry /= 58;
        }
        digits_len = j;
    }

    std::string output;
    output.reserve(zeros + digits_len);
    output.assign(zeros, '1');
    for (auto it = digits.end() - digits_len; it != digits.end(); ++it)
    {
        output.push_back(ALPHABET[*it]);
    }
    return output;
}

bool Base58::decode(std::string const & input, std::vector<uint8_t> & output)
{
    return decode(input.c_str(), output);
}

bool Base58::decode(char const * input, std::vector<uint8_t> & output)
{
    output.clear();
    if (input == nullptr)
    {
        return false;
    }

    // Count and skip leading '1' characters (each = one zero byte).
    size_t zeros = 0;
    while (*input == '1')
    {
        ++zeros;
        ++input;
    }

    size_t const input_len = std::strlen(input);
    // Worst-case byte count: ceil(N * log(58) / log(256)) ~= N * 733 / 1000 + 1.
    size_t const bytes_capacity = input_len * 733 / 1000 + 1;
    std::vector<uint8_t> bytes(bytes_capacity, 0);
    size_t bytes_len = 0;

    for (size_t i = 0; i < input_len; ++i)
    {
        int digit = decodeChar(input[i]);
        if (digit < 0)
        {
            return false;
        }

        int carry = digit;
        size_t j = 0;
        for (auto it = bytes.rbegin(); (carry != 0 || j < bytes_len) && it != bytes.rend(); ++it, ++j)
        {
            carry += 58 * (*it);
            *it = static_cast<uint8_t>(carry & 0xff);
            carry >>= 8;
        }
        bytes_len = j;
    }

    output.assign(zeros, 0);
    output.insert(output.end(), bytes.end() - bytes_len, bytes.end());
    return true;
}
