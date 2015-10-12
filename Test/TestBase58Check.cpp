#include "equity/Base58Check.h"
#include "equity/Utility.h"

#include <string>
#include <vector>

using namespace Equity;

int TestBase58Check()
{
    int errors = 0;

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
            ++errors;
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
            ++errors;
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
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, Utility::vtox(CASES[i].data, CASES[i].size).c_str(), Utility::vtox(resultVector).c_str());
            ++errors;
        }
    }

    return errors;
}
