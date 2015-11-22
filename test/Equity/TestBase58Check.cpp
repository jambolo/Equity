#include "equity/Base58Check.h"
#include "utility/Utility.h"

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

    printf("+-- Base58Check\n");

    printf("    +-- testing: std::string encode(uint8_t const * input, size_t length, unsigned version)\n");
    for (auto const & c : CASES)
    {
        std::string name = Utility::shorten(c.stringForm);
        std::string result = Base58Check::encode(c.data, c.size, c.version);
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

    printf("    +-- testing: bool decode(char const * input, std::vector<uint8_t> & output, unsigned & version)\n");
    for (auto const & c : DECODE_VALIDITY_CASES)
    {
        std::string name = Utility::shorten(c.data);
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(c.data, resultVector, resultVersion);
        if (ok != c.expected)
        {
            printf("        +== %s: expected \"%s\"\n", name.c_str(), c.expected ? "true" : "false");
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }
    for (auto const & c : CASES)
    {
        std::string name = Utility::shorten(Utility::toHex(c.data, c.size));
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(c.stringForm, resultVector, resultVersion);
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
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), Utility::toHex(c.data, c.size).c_str(), Utility::toHex(resultVector).c_str());
            ++errors;
        }
        else if (resultVersion != c.version)
        {
            printf("        +== %s: expected version %d, got %d\n", name.c_str(), c.version, resultVersion);
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    return errors;
}
