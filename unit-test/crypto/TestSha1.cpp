#include "../targetver.h"
#include "CppUnitTest.h"

#include "crypto/Sha1.h"
#include "utility/Utility.h"
#include <cstdio>

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{

struct Sha1TestCase
{
    char const * input;
    uint8_t expected[SHA1_HASH_SIZE];
};

Sha1TestCase const SHA1_CASES[] =
{
    {
        "",
        { 0xda, 0x39, 0xa3, 0xee, 0x5e, 0x6b, 0x4b, 0x0d, 0x32, 0x55, 0xbf, 0xef, 0x95, 0x60, 0x18, 0x90, 0xaf, 0xd8, 0x07, 0x09 }
    },
    {
        "a",
        { 0x86, 0xf7, 0xe4, 0x37, 0xfa, 0xa5, 0xa7, 0xfc, 0xe1, 0x5d, 0x1d, 0xdc, 0xb9, 0xea, 0xea, 0xea, 0x37, 0x76, 0x67, 0xb8 }
    },
    {
        "abc",
        { 0xa9, 0x99, 0x3e, 0x36, 0x47, 0x06, 0x81, 0x6a, 0xba, 0x3e, 0x25, 0x71, 0x78, 0x50, 0xc2, 0x6c, 0x9c, 0xd0, 0xd8, 0x9d }
    },
    {
        "message digest",
        { 0xc1, 0x22, 0x52, 0xce, 0xda, 0x8b, 0xe8, 0x99, 0x4d, 0x5f, 0xa0, 0x29, 0x0a, 0x47, 0x23, 0x1c, 0x1d, 0x16, 0xaa, 0xe3 }
    },
    {
        "abcdefghijklmnopqrstuvwxyz",
        { 0x32, 0xd1, 0x0c, 0x7b, 0x8c, 0xf9, 0x65, 0x70, 0xca, 0x04, 0xce, 0x37, 0xf2, 0xa1, 0x9d, 0x84, 0x24, 0x0d, 0x3a, 0x89 }
    },
    {
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        { 0x84, 0x98, 0x3e, 0x44, 0x1c, 0x3b, 0xd2, 0x6e, 0xba, 0xae, 0x4a, 0xa1, 0xf9, 0x51, 0x29, 0xe5, 0xe5, 0x46, 0x70, 0xf1 }
    },
    {
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789",
        { 0x76, 0x1c, 0x45, 0x7b, 0xf7, 0x3b, 0x14, 0xd2, 0x7e, 0x9e, 0x92, 0x65, 0xc4, 0x6f, 0x4b, 0x4d, 0xda, 0x11, 0xf9, 0x40 }
    },
    {
        "12345678901234567890123456789012345678901234567890123456789012345678901234567890",
        { 0x50, 0xab, 0xf5, 0x70, 0x6a, 0x15, 0x09, 0x90, 0xa0, 0x8b, 0x2c, 0x5e, 0xa4, 0x0f, 0xa0, 0xe5, 0x85, 0x55, 0x47, 0x32 }
    }
};

} // anonymous namespace

namespace TestCrypto
{
    
TEST_CLASS(Crypto_Sha1)
{
public:
    TEST_METHOD(Crypto_Sha1_sha1_ptr)
    {
        for (auto const & c : SHA1_CASES)
        {
            Sha1Hash result = Crypto::sha1((uint8_t const *)c.input, strlen(c.input));
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"sha1", c.input, c.expected, sizeof(c.expected), &result[0], result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha1_sha1_vector)
    {
        for (auto const & c : SHA1_CASES)
        {
            std::vector<uint8_t> input((uint8_t const *)c.input, (uint8_t const *)c.input+strlen(c.input));
            Sha1Hash result = Crypto::sha1(input);
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"sha1", c.input, c.expected, sizeof(c.expected), &result[0], result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha1_sha1_array)
    {
    }
    
    static std::wstring hashErrorMessage(wchar_t const * test,
                                         char const *    input,
                                         uint8_t const * expected,
                                         size_t          expectedSize,
                                         uint8_t const * actual,
                                         size_t          actualSize)
    {
        std::wostringstream message;
        message
            << test
            << L"(\""
            << ToString(shorten(input))
            << L"\"): expected "
            << ToString(toHex(expected, expectedSize))
            << ", got "
            << ToString(toHex(actual, actualSize));
        return message.str();
    }
};

} // namespace TestCrypto
