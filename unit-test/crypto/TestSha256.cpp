#include "../targetver.h"
#include "CppUnitTest.h"

#include "crypto/Sha256.h"
#include "utility/Utility.h"
#include <cstdio>

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{

struct Sha256TestCase
{
    char const * input;
    uint8_t expected[SHA256_HASH_SIZE];
};

Sha256TestCase const SHA256_CASES[] =
{
    {
        "",
        {
            0xe3, 0xb0, 0xc4, 0x42, 0x98, 0xfc, 0x1c, 0x14, 0x9a, 0xfb, 0xf4, 0xc8, 0x99, 0x6f, 0xb9, 0x24,
            0x27, 0xae, 0x41, 0xe4, 0x64, 0x9b, 0x93, 0x4c, 0xa4, 0x95, 0x99, 0x1b, 0x78, 0x52, 0xb8, 0x55
        }
    },
    {
        "abc",
        {
            0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea, 0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
            0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c, 0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad
        }
    },
    {
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        {
            0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8, 0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
            0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67, 0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1
        }
    },
    {
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
        {
            0xcf, 0x5b, 0x16, 0xa7, 0x78, 0xaf, 0x83, 0x80, 0x03, 0x6c, 0xe5, 0x9e, 0x7b, 0x04, 0x92, 0x37,
            0x0b, 0x24, 0x9b, 0x11, 0xe8, 0xf0, 0x7a, 0x51, 0xaf, 0xac, 0x45, 0x03, 0x7a, 0xfe, 0xe9, 0xd1
        }
    }
};


struct DoubleSha256TestCase
{
    char const * input;
    uint8_t expected[SHA256_HASH_SIZE];
};

DoubleSha256TestCase const DOUBLE_SHA256_CASES[] =
{
    {
        "",
        {
            0x5d, 0xf6, 0xe0, 0xe2, 0x76, 0x13, 0x59, 0xd3, 0x0a, 0x82, 0x75, 0x05, 0x8e, 0x29, 0x9f, 0xcc,
            0x03, 0x81, 0x53, 0x45, 0x45, 0xf5, 0x5c, 0xf4, 0x3e, 0x41, 0x98, 0x3f, 0x5d, 0x4c, 0x94, 0x56
        }
    },
    {
        "abc",
        {
            0x4f, 0x8b, 0x42, 0xc2, 0x2d, 0xd3, 0x72, 0x9b, 0x51, 0x9b, 0xa6, 0xf6, 0x8d, 0x2d, 0xa7, 0xcc,
            0x5b, 0x2d, 0x60, 0x6d, 0x05, 0xda, 0xed, 0x5a, 0xd5, 0x12, 0x8c, 0xc0, 0x3e, 0x6c, 0x63, 0x58
        }
    },
    {
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq",
        {
            0x0c, 0xff, 0xe1, 0x7f, 0x68, 0x95, 0x4d, 0xac, 0x3a, 0x84, 0xfb, 0x14, 0x58, 0xbd, 0x5e, 0xc9,
            0x92, 0x09, 0x44, 0x97, 0x49, 0xb2, 0xb3, 0x08, 0xb7, 0xcb, 0x55, 0x81, 0x2f, 0x95, 0x63, 0xaf
        }
    },
    {
        "abcdefghbcdefghicdefghijdefghijkefghijklfghijklmghijklmnhijklmnoijklmnopjklmnopqklmnopqrlmnopqrsmnopqrstnopqrstu",
        {
            0xac, 0xcd, 0x7b, 0xd1, 0xcb, 0x0f, 0xcb, 0xd8, 0x5c, 0xf0, 0xba, 0x5b, 0xa9, 0x69, 0x45, 0x12,
            0x77, 0x76, 0x37, 0x3a, 0x7d, 0x47, 0x89, 0x1e, 0xb4, 0x3e, 0xd6, 0xb1, 0xe2, 0xee, 0x60, 0xfe
        }
    }
};

} // anonymous namespace

namespace TestCrypto
{
    
TEST_CLASS(Crypto_Sha256)
{
public:
    TEST_METHOD(Crypto_Sha256_sha256_ptr)
    {
        for (auto const & c : SHA256_CASES)
        {
            Sha256Hash result = Crypto::sha256((uint8_t const *)c.input, strlen(c.input));
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"sha256", c.input, c.expected, sizeof(c.expected), &result[0], result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha256_sha256_vector)
    {
        for (auto const & c : SHA256_CASES)
        {
            std::vector<uint8_t> input((uint8_t const *)c.input, (uint8_t const *)c.input+strlen(c.input));
            Sha256Hash result = Crypto::sha256(input);
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"sha256", c.input, c.expected, sizeof(c.expected), &result[0], result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha256_sha256_array)
    {
    }
    
    TEST_METHOD(Crypto_Sha256_doubleSha256_ptr)
    {
        for (auto const & c : DOUBLE_SHA256_CASES)
        {
            Sha256Hash result = Crypto::doubleSha256((uint8_t const *)c.input, strlen(c.input));
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"doubleSha256", c.input, c.expected, sizeof(c.expected), &result[0],
                                            result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha256_doubleSha256_vector)
    {
        for (auto const & c : DOUBLE_SHA256_CASES)
        {
            std::vector<uint8_t> input((uint8_t const *)c.input, (uint8_t const *)c.input+strlen(c.input));
            Sha256Hash result = Crypto::doubleSha256(input);
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"doubleSha256", c.input, c.expected, sizeof(c.expected), &result[0],
                                            result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha256_checksum_ptr)
    {
        for (auto const & c : DOUBLE_SHA256_CASES)
        {
            Checksum result = Crypto::checksum((uint8_t const *)c.input, strlen(c.input));
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"checksum", c.input, c.expected, CHECKSUM_SIZE, &result[0], result.size()).c_str());
        }
    }
    
    TEST_METHOD(Crypto_Sha256_checksum_vector)
    {
        for (auto const & c : DOUBLE_SHA256_CASES)
        {
            std::vector<uint8_t> input((uint8_t const *)c.input, (uint8_t const *)c.input+strlen(c.input));
            Checksum result = Crypto::checksum((input);
            Assert::IsTrue(std::equal(result.begin(), result.end(), c.expected),
                           hashErrorMessage(L"checksum", c.input, c.expected, CHECKSUM_SIZE, &result[0], result.size()).c_str());
        }
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
