#include "crypto/Ripemd.h"
#include "equity/Address.h"
#include "equity/Base58.h"
#include "equity/Base58Check.h"
#include "equity/Utility.h"

#include <string>
#include <vector>

using namespace Crypto;
using namespace Equity;

static void TestBase58();
static void TestBase58Check();
static void TestPublicKey();
static void TestAddress();

struct Test
{
    char const * name;
    void(*f)();
};
static Test TESTS[] =
{
    { "Base58",         TestBase58 },
    { "Base58Check",    TestBase58Check },
    { "PublicKey",      TestPublicKey },
    { "Address",        TestAddress }

};
static size_t const TESTS_SIZE = sizeof(TESTS) / sizeof(Test);

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i)
    {
        char * name = argv[i];
        for (int j = 0; j < TESTS_SIZE; ++j)
        {
            if (strcmp(name, TESTS[j].name) == 0)
            {
                TESTS[j].f();
                break;
            }
        }
    }

    return 0;
}

static void TestBase58()
{
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
    static size_t const ENCODE_CASES_SIZE = sizeof(ENCODE_CASES) / sizeof(TestInput);

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
    static size_t const DECODE_CASES_SIZE = sizeof(DECODE_CASES) / sizeof(TestInput);

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
    static size_t const DECODE_VALIDITY_CASES_SIZE = sizeof(DECODE_VALIDITY_CASES) / sizeof(DecodeValidityTestInput);

    printf("Base58\n");

    printf("+-- testing: std::string encode(uint8_t const * input, size_t length)\n");
    for (int i = 0; i < ENCODE_CASES_SIZE; ++i)
    {
        std::string result = Base58::encode(ENCODE_CASES[i].data, ENCODE_CASES[i].size);
        if (result == ENCODE_CASES[i].stringForm)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, ENCODE_CASES[i].stringForm, result.c_str());
        }
    }

    printf("+-- testing: bool decode(char const * input, std::vector<uint8_t> & output)\n");
    for (int i = 0; i < DECODE_VALIDITY_CASES_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;

        bool ok = Base58::decode(DECODE_VALIDITY_CASES[i].data, resultVector);
        if (ok == DECODE_VALIDITY_CASES[i].expected)
        {
            printf("    +-- %c: ok\n", 'A' + i);
        }
        else
        {
            printf("    +-- %c: error, expected \"%s\" when decoding \"%s\"\n", 'A' + i, DECODE_VALIDITY_CASES[i].expected ? "true" : "false", DECODE_VALIDITY_CASES[i].data);
        }
    }
    for (int i = 0; i < DECODE_CASES_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;

        bool ok = Base58::decode(DECODE_CASES[i].stringForm, resultVector);
        if (ok &&
            resultVector.size() == DECODE_CASES[i].size &&
            std::equal(resultVector.begin(), resultVector.end(), DECODE_CASES[i].data))
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, Utility::vtox(std::vector<uint8_t>(DECODE_CASES[i].data, DECODE_CASES[i].data + DECODE_CASES[i].size)).c_str(), Utility::vtox(resultVector).c_str());
        }
    }
}

static void TestBase58Check()
{
    struct TestInput
    {
        unsigned version;
        size_t size;
        uint8_t data[256];
        char const * stringForm;
    };
    static unsigned const MAIN_NETWORK_ID = 0;
    static unsigned const BITCOIN_PRIVATE_KEY_VERSION = 0x80;
    static TestInput const CASES[] =
    {
        { MAIN_NETWORK_ID, 20, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1111111111111111111114oLvT2" },
        { MAIN_NETWORK_ID, 20, { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE }, "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM" },
        { MAIN_NETWORK_ID, 20, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr" },
        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf" },
        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0xA1, 0xD8, 0x50, 0x84, 0x5A, 0x07, 0x76, 0xE0, 0xC8, 0x59, 0x64, 0x4A, 0x67, 0x3F, 0xAF, 0x7A, 0x55, 0x2E, 0x0B, 0x76, 0xEE, 0xFF, 0xAA, 0x91, 0x3E, 0xEF, 0xB7, 0x7E, 0x55, 0xE8, 0x19, 0x6A }, "5K3ZhDmm48hRrtHHv1SX9P8bszygzDk75EwnjfB5rJeVSzYCbsp" },
        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40 }, "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetqj84qw" }
    };
    static size_t const CASES_SIZE = sizeof(CASES) / sizeof(TestInput);
    struct DecodeValidityTestInput
    {
        char const * data;
        bool expected;
    };
    static DecodeValidityTestInput const DECODE_VALIDITY_CASES[] =
    {
        { "", false },
        { "1", false },
        { "11", false },
        { "112", false },
        { "112e", false },
        { "112ed", false },
        { "112edB", false },
        { "112edB6", false },
        { "112edB6q", true },
    };
    static size_t const DECODE_VALIDITY_CASES_SIZE = sizeof(DECODE_VALIDITY_CASES) / sizeof(DecodeValidityTestInput);

    printf("Base58Check\n");

    printf("+-- testing: std::string encode(uint8_t const * input, size_t length, unsigned version)\n");
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        std::string result = Base58Check::encode(CASES[i].data, CASES[i].size, CASES[i].version);
        if (result == CASES[i].stringForm)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, CASES[i].stringForm, result.c_str());
        }
    }

    printf("+-- testing: bool decode(char const * input, std::vector<uint8_t> & output, unsigned & version)\n");
    for (int i = 0; i < DECODE_VALIDITY_CASES_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(DECODE_VALIDITY_CASES[i].data, resultVector, resultVersion);
        if (ok == DECODE_VALIDITY_CASES[i].expected)
        {
            printf("    +-- %c: ok\n", 'A' + i);
        }
        else
        {
            printf("    +-- %c: error, expected \"%s\" when decoding \"%s\"\n", 'A' + i, DECODE_VALIDITY_CASES[i].expected ? "true" : "false", DECODE_VALIDITY_CASES[i].data);
        }
    }
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(CASES[i].stringForm, resultVector, resultVersion);
        if (ok &&
            resultVector.size() == CASES[i].size &&
            std::equal(resultVector.begin(), resultVector.end(), CASES[i].data) &&
            resultVersion == CASES[i].version)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, Utility::vtox(std::vector<uint8_t>(CASES[i].data, CASES[i].data + CASES[i].size)).c_str(), Utility::vtox(resultVector).c_str());
        }
    }
}


static void TestPublicKey()
{
    //explicit PublicKey(std::vector<uint8_t> const & k);
    //explicit PublicKey(uint8_t const * k);
    //explicit PublicKey(PrivateKey const & k);

    //std::vector<uint8_t> value() const  { return value_; }
    //bool valid() const                  { return valid_; }

    //std::string toHex() const;

}

static void TestAddress()
{
    struct TestInput
    {
        unsigned version;
        size_t size;
        uint8_t data[256];
        char const * stringForm;
    };
    static unsigned const MAIN_NETWORK_ID = 0;
    static unsigned const TEST_NETWORK_ID = 0xc4;
    static unsigned const P2SH_NETWORK_ID = 5;
    static TestInput const CASES[] =
    {
        { MAIN_NETWORK_ID, 20, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1111111111111111111114oLvT2" },
        { MAIN_NETWORK_ID, 20, { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE }, "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM" },
        { MAIN_NETWORK_ID, 20, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr" },
        { TEST_NETWORK_ID, 20, { 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE, 0x01 }, "2Mt6vpVdNZziX6xefS5BWZQ5PjGH9cabz7v" },
        { P2SH_NETWORK_ID, 20, { 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE, 0x01, 0x09 }, "3B2orSMAV7jhCgubX9NphAA2uXkFkeJ61y" },
    };
    static size_t const CASES_SIZE = sizeof(CASES) / sizeof(TestInput);

    printf("Address\n");

    //explicit Address(PublicKey const & publicKey);
    //explicit Address(PrivateKey const & privateKey);

    //std::string toString(unsigned network) const;

    struct StringValidityTestInput
    {
        char const * data;
        bool expected;
    };
    static StringValidityTestInput const STRING_VALIDITY_CASES[] =
    {
        { "", false },
        { "112edB6q", false },                              // minimum base58check, but too short
        { "137kV53f3mW571W38VFn9WBSmrx1eiMNy", false },     // one byte too short
        { "1iEMistU6HpJwnLgUixyZYvURF28JGhxvSx", false }    // one byte too big
    };
    static size_t const STRING_VALIDITY_CASES_SIZE = sizeof(STRING_VALIDITY_CASES) / sizeof(StringValidityTestInput);

    printf("+-- testing: Address(uint8_t const * k)\n");
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        Address result(CASES[i].data);
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == CASES[i].size &&
            std::equal(value.begin(), value.end(), CASES[i].data) &&
            result.toString(CASES[i].version) == CASES[i].stringForm)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, CASES[i].stringForm, result.toString(CASES[i].version).c_str());
        }
    }

    printf("+-- testing: Address(std::string const & s)\n");
    for (int i = 0; i < STRING_VALIDITY_CASES_SIZE; ++i)
    {
        Address result(STRING_VALIDITY_CASES[i].data);
        if (result.valid() == STRING_VALIDITY_CASES[i].expected)
        {
            printf("    +-- %c: ok\n", 'A' + i);
        }
        else
        {
            printf("    +-- %c: error, expected \"%s\" when constructing \"%s\"\n", 'A' + i, STRING_VALIDITY_CASES[i].expected ? "true" : "false", STRING_VALIDITY_CASES[i].data);
        }
    }
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(CASES[i].stringForm, resultVector, resultVersion);
        if (ok &&
            resultVector.size() == CASES[i].size &&
            std::equal(resultVector.begin(), resultVector.end(), CASES[i].data) &&
            resultVersion == CASES[i].version)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, Utility::vtox(std::vector<uint8_t>(CASES[i].data, CASES[i].data + CASES[i].size)).c_str(), Utility::vtox(resultVector).c_str());
        }
    }
}