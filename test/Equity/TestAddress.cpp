#include "equity/Address.h"
#include "equity/PublicKey.h"
#include "utility/Utility.h"

#include <string>
#include <vector>
#include <cstdio>

using namespace Equity;

int TestAddress()
{
    int errors = 0;

    printf("+-- Address\n");

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

    printf("    +-- testing: Address(uint8_t const * k)\n");

    for (int i = 0; i < CASES_SIZE; ++i)
    {
        Address result(CASES[i].data);
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == CASES[i].size &&
            std::equal(value.begin(), value.end(), CASES[i].data) &&
            result.toString(CASES[i].version) == CASES[i].stringForm)
        {
            printf("        +-- %d: ok\n", i);
        }
        else
        {
            printf("        +-- %d: expected \"%s\", got \"%s\"\n", i, CASES[i].stringForm, result.toString(CASES[i].version).c_str());
            ++errors;
        }
    }

    printf("    +-- testing: Address(std::string const & s)\n");

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
        { "1iEMistU6HpJwnLgUixyZYvURF28JGhxvSx", false },   // one byte too big
        { "1BadxxxxxxxxxxxxxxxxxxxxxxxvvKy2F", false }      // invalid checksum
    };
    static size_t const STRING_VALIDITY_CASES_SIZE = sizeof(STRING_VALIDITY_CASES) / sizeof(StringValidityTestInput);

    for (int i = 0; i < STRING_VALIDITY_CASES_SIZE; ++i)
    {
        Address result(STRING_VALIDITY_CASES[i].data);
        if (result.valid() == STRING_VALIDITY_CASES[i].expected)
        {
            printf("        +-- %c: ok\n", 'A' + i);
        }
        else
        {
            printf("        +-- %c: expected \"%s\" when constructing \"%s\"\n", 'A' + i, STRING_VALIDITY_CASES[i].expected ? "true" : "false", STRING_VALIDITY_CASES[i].data);
            ++errors;
        }
    }
    for (int i = 0; i < CASES_SIZE; ++i)
    {
        Address result(CASES[i].stringForm);
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == CASES[i].size &&
            std::equal(value.begin(), value.end(), CASES[i].data))
        {
            printf("        +-- %d: ok\n", i);
        }
        else
        {
            printf("        +-- %d: expected \"%s\", got \"%s\"\n", i, Utility::vtox(std::vector<uint8_t>(CASES[i].data, CASES[i].data + CASES[i].size)).c_str(), Utility::vtox(value).c_str());
            ++errors;
        }
    }

    printf("    +-- testing: Address(PublicKey const & publicKey)\n");

    struct PublicKeyInput
    {
        uint8_t publicKey[PublicKey::SIZE];
        char const * address;
    };

    static PublicKeyInput PUBLIC_KEY_CASES[] =
    {
        { { 0x04,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
            },
            "16QaFeudRUt8NYy2yzjm3BMvG4xBbAsBFM"
        },
        { { 0x04,
        0x78, 0xD4, 0x30, 0x27, 0x4F, 0x8C, 0x5E, 0xC1, 0x32, 0x13, 0x38, 0x15, 0x1E, 0x9F, 0x27, 0xF4,
        0xC6, 0x76, 0xA0, 0x08, 0xBD, 0xF8, 0x63, 0x8D, 0x07, 0xC0, 0xB6, 0xBE, 0x9A, 0xB3, 0x5C, 0x71,
        0xA1, 0x51, 0x80, 0x63, 0x24, 0x3A, 0xCD, 0x4D, 0xFE, 0x96, 0xB6, 0x6E, 0x3F, 0x2E, 0xC8, 0x01,
        0x3C, 0x8E, 0x07, 0x2C, 0xD0, 0x9B, 0x38, 0x34, 0xA1, 0x9F, 0x81, 0xF6, 0x59, 0xCC, 0x34, 0x55
            },
            "1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T"
        }
    };
    static size_t const PUBLIC_KEY_CASES_SIZE = sizeof(PUBLIC_KEY_CASES) / sizeof(PublicKeyInput);

    for (int i = 0; i < PUBLIC_KEY_CASES_SIZE; ++i)
    {
        Address result(PublicKey(PUBLIC_KEY_CASES[i].publicKey));
        std::vector<uint8_t> value = result.value();

        if (result.valid() &&
            value.size() == Address::SIZE &&
            result.toString(MAIN_NETWORK_ID) == PUBLIC_KEY_CASES[i].address)
        {
            printf("        +-- %d: ok\n", i);
        }
        else
        {
            printf("        +-- %d: expected \"%s\", got \"%s\"\n", i, PUBLIC_KEY_CASES[i].address, result.toString(MAIN_NETWORK_ID).c_str());
            ++errors;
        }
    }

    //explicit Address(PrivateKey const & privateKey);

    return errors;
}
