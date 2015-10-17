#include "equity/PrivateKey.h"
#include "utility/Utility.h"

#include <string>
#include <vector>
#include <cstdio>

using namespace Equity;

static unsigned const BITCOIN_PRIVATE_KEY_VERSION = 0x80;

int TestPrivateKey()
{
    int errors = 0;

    printf("PrivateKey\n");

    struct Input
    {
        uint8_t data[32];
        char const * wif;
    };

    static Input CASES[] =
    {
        {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
            "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf"
        },
        {
            {
                0x6E, 0x12, 0x0E, 0xA6, 0x23, 0xBC, 0xAB, 0x6E, 0x77, 0xD4, 0x63, 0x0B, 0x44, 0x2E, 0x3D, 0x98,
                0x3F, 0xAE, 0x17, 0xF1, 0x7D, 0xA6, 0xF0, 0x59, 0x20, 0x3A, 0x4C, 0x37, 0x1E, 0xD3, 0x29, 0x8E
            },
            "5JemBigghFJRCydhheaeNbeHqDnYmpxJMiNEsyMUyyZmAfpwoUM"
        },
        {
            {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
                0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
            },
            "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetqj84qw"
        }
    };

    static size_t const CASES_SIZE = sizeof(CASES) / sizeof(Input);

    printf("+-- testing: PrivateKey(uint8_t const * k)\n");

    for (int i = 0; i < CASES_SIZE; ++i)
    {
        PrivateKey result(CASES[i].data);
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == PrivateKey::SIZE &&
            std::equal(value.begin(), value.end(), CASES[i].data) &&
            result.toWif(BITCOIN_PRIVATE_KEY_VERSION) == CASES[i].wif)
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, CASES[i].wif, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());
            ++errors;
        }
    }

    printf("+-- testing: PrivateKey(std::string const & wif)\n");

    struct StringValidityTestInput
    {
        char const * data;
        bool expected;
    };
    static StringValidityTestInput const STRING_VALIDITY_CASES[] =
    {
        { "", false },
        { "26k9aD1PF", false },                                             // minimum base58check, but too short
        { "yg26Xdu6SVmmVddThR8kwpf49TZ8UT9WDTLbM1FpLPzvDafte", false },     // one byte too short
        { "15JtGrFPM4GihbattjeNy4rXeL7GGhurMeqbubbepp48jUGJfj8G", false },   // one byte too big
        { "5K9DqQDrw1XzbPNfvqPXExyLxtLCozAQXwyFUbJ8PvcWPMx5bja", false }    // checksum is wrong
    };
    static size_t const STRING_VALIDITY_CASES_SIZE = sizeof(STRING_VALIDITY_CASES) / sizeof(StringValidityTestInput);

    for (int i = 0; i < STRING_VALIDITY_CASES_SIZE; ++i)
    {
        PrivateKey result(STRING_VALIDITY_CASES[i].data);
        if (result.valid() == STRING_VALIDITY_CASES[i].expected)
        {
            printf("    +-- %c: ok\n", 'A' + i);
        }
        else
        {
            printf("    +-- %c: error, expected \"%s\" when constructing \"%s\"\n", 'A' + i, STRING_VALIDITY_CASES[i].expected ? "true" : "false", STRING_VALIDITY_CASES[i].data);
            ++errors;
        }
    }
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        PrivateKey result(CASES[i].wif);
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == PrivateKey::SIZE &&
            std::equal(value.begin(), value.end(), CASES[i].data))
        {
            printf("    +-- %d: ok\n", i);
        }
        else
        {
            printf("    +-- %d: error, expected \"%s\", got \"%s\"\n", i, Utility::vtox(CASES[i].data, PrivateKey::SIZE).c_str(), Utility::vtox(value).c_str());
            ++errors;
        }
    }

    return errors;
}
