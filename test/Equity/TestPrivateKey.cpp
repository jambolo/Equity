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

    printf("+-- PrivateKey\n");

    struct Input
    {
        uint8_t data[32];
        char const * wif;
        char const * wifCompressed;
    };

    static Input CASES[] =
    {
        {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
            "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf",
            "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn"
        },
        {
            {
                0x6E, 0x12, 0x0E, 0xA6, 0x23, 0xBC, 0xAB, 0x6E, 0x77, 0xD4, 0x63, 0x0B, 0x44, 0x2E, 0x3D, 0x98,
                0x3F, 0xAE, 0x17, 0xF1, 0x7D, 0xA6, 0xF0, 0x59, 0x20, 0x3A, 0x4C, 0x37, 0x1E, 0xD3, 0x29, 0x8E
            },
            "5JemBigghFJRCydhheaeNbeHqDnYmpxJMiNEsyMUyyZmAfpwoUM",
            "Kzug2JzACe7sqrxzeQsVxKubHqS3Erf5cRakFyjQWUfbwbm1hBUS"
        },
        {
            {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
                0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
            },
            "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetqj84qw",
            "L5oLkpV3aqBjhki6LmvChTCV6odsp4SXM6FfU2Gppt5kFLaHLuZ9"
        }
    };

    printf("    +-- testing: PrivateKey(uint8_t const * k)\n");

    for (auto c : CASES)
    {
        std::string name = Utility::shorten(Utility::toHex(c.data, 32));
        PrivateKey result(c.data);
        std::vector<uint8_t> value = result.value();

        if (!result.valid())
        {
            printf("        +== %s: not valid\n", name.c_str());
            ++errors;
        }
        else if (value.size() != PrivateKey::SIZE)
        {
            printf("        +== %s: expected size = %u, got size = %u", name.c_str(), (unsigned)PrivateKey::SIZE, (unsigned)value.size());
            ++errors;
        }
        else if (!std::equal(value.begin(), value.end(), c.data))
        {
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), Utility::toHex(c.data, 32).c_str(), Utility::toHex(value).c_str());
            ++errors;
        }
        else if (result.toWif(BITCOIN_PRIVATE_KEY_VERSION) != c.wif)
        {
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), c.wif, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());
            ++errors;
        }
        else if (result.compressed())
        {
            printf("        +== %s: expected not compressed, got compressed\n", name.c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: PrivateKey(std::string const & wif) (validity)\n");

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
        { "15JtGrFPM4GihbattjeNy4rXeL7GGhurMeqbubbepp48jUGJfj8G", false },  // one byte too big
        { "5K9DqQDrw1XzbPNfvqPXExyLxtLCozAQXwyFUbJ8PvcWPMx5bja", false }    // checksum is wrong
    };

    for (auto c : STRING_VALIDITY_CASES)
    {
        std::string name = Utility::shorten(c.data);
        PrivateKey result(c.data);
        if (result.valid() != c.expected)
        {
            printf("        +== %s: expected valid = \"%s\"\n", name.c_str(), c.expected ? "true" : "false");
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: PrivateKey(std::string const & wif) (uncompressed)\n");

    for (auto c : CASES)
    {
        std::string name = Utility::shorten(c.wif);
        PrivateKey result(c.wif);
        std::vector<uint8_t> value = result.value();

        if (!result.valid())
        {
            printf("        +== %s: not valid\n", name.c_str());
            ++errors;
        }
        else if (value.size() != PrivateKey::SIZE)
        {
            printf("        +== %s: expected size = %u, got size = %u\n", name.c_str(), (unsigned)PrivateKey::SIZE, (unsigned)value.size());
            ++errors;
        }
        else if (result.compressed())
        {
            printf("        +== %s: expected not compressed, got compressed\n", name.c_str());
            ++errors;
        }
        else if (!std::equal(value.begin(), value.end(), c.data))
        {
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), Utility::toHex(c.data, PrivateKey::SIZE).c_str(), Utility::toHex(value).c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: PrivateKey(std::string const & wif) (compressed)\n");

    for (auto c : CASES)
    {
        std::string name = Utility::shorten(c.wifCompressed);
        PrivateKey result(c.wifCompressed);

        std::vector<uint8_t> value = result.value();

        if (!result.valid())
        {
            printf("        +== %s: not valid\n", name.c_str());
            ++errors;
        }
        else if (value.size() != PrivateKey::SIZE)
        {
            printf("        +== %s: expected size = %u, got size = %u\n", name.c_str(), (unsigned)PrivateKey::SIZE, (unsigned)value.size());
            ++errors;
        }
        else if (!result.compressed())
        {
            printf("        +== %s: expected compressed, got not compressed\n", name.c_str());
            ++errors;
        }
        else if (!std::equal(value.begin(), value.end(), c.data))
        {
            printf("        +== %s: expected \"%s\", got \"%s\"\n", name.c_str(), Utility::toHex(c.data, PrivateKey::SIZE).c_str(), Utility::toHex(value).c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: setCompressed(false)\n");

    for (auto c : CASES)
    {
        std::string name = Utility::shorten(c.wif);
        PrivateKey result(c.data);
        result.setCompressed(false);

        if (result.compressed())
        {
            printf("        +== %s: expected not compressed, got compressed\n", name.c_str());
            ++errors;
        }
        else if (result.toWif(BITCOIN_PRIVATE_KEY_VERSION) != c.wif)
        {
            printf("        +== %s: got \"%s\"\n", name.c_str(), result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }

    printf("    +-- testing: setCompressed(true)\n");

    for (auto c : CASES)
    {
        std::string name = Utility::shorten(c.wif);
        PrivateKey result(c.data);
        result.setCompressed(true);

        if (!result.compressed())
        {
            printf("        +-- %s: expected compressed, got not compressed\n", name.c_str());
            ++errors;
        }
        else if (result.toWif(BITCOIN_PRIVATE_KEY_VERSION) != c.wifCompressed)
        {
            printf("        +-- %s: expected \"%s\", got \"%s\"\n", name.c_str(), c.wifCompressed, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());
            ++errors;
        }
        else
        {
            printf("        +-- %s: ok\n", name.c_str());
        }
    }


    return errors;
}
