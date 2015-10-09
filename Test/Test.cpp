#include "equity/Base58.h"
#include "equity/Base58Check.h"
#include "crypto/Ripemd.h"

#include <string>

using namespace Crypto;
using namespace Equity;

static void TestBase58Check();

struct Test
{
    char const * name;
    void (*f)();
};
static Test TESTS[] =
{
    { "TestBase58Check", TestBase58Check }
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

void TestBase58Check()
{
    struct TestInput
    {
        unsigned version;
        size_t size;
        uint8_t hash[256];
        char const * stringVersion;
    };
    static unsigned const BITCOIN_ADDRESS_VERSION = 0;
    static unsigned const BITCOIN_PRIVATE_KEY_VERSION = 0x80;
    static TestInput const DATA[] =
    {
        { BITCOIN_ADDRESS_VERSION, 20, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1111111111111111111114oLvT2" },
        { BITCOIN_ADDRESS_VERSION, 20, { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE }, "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM" },
        { BITCOIN_ADDRESS_VERSION, 20, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }, "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr" },

        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 }, "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf" },
        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0xA1, 0xD8, 0x50, 0x84, 0x5A, 0x07, 0x76, 0xE0, 0xC8, 0x59, 0x64, 0x4A, 0x67, 0x3F, 0xAF, 0x7A, 0x55, 0x2E, 0x0B, 0x76, 0xEE, 0xFF, 0xAA, 0x91, 0x3E, 0xEF, 0xB7, 0x7E, 0x55, 0xE8, 0x19, 0x6A }, "5K3ZhDmm48hRrtHHv1SX9P8bszygzDk75EwnjfB5rJeVSzYCbsp" },
        { BITCOIN_PRIVATE_KEY_VERSION, 32, { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40 }, "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetqj84qw" }
    };
    static size_t const DATA_SIZE = sizeof(DATA) / sizeof(TestInput);

    printf("TestBase58Check\n");

    for (int i = 0; i < DATA_SIZE; ++i)
    {
        std::string result = Base58Check::encode(DATA[i].hash, DATA[i].size, DATA[i].version);
        if (result == DATA[i].stringVersion)
        {
            printf("encode %d: ok\n", i);
        }
        else
        {
            printf("encode %d: error\n", i);
        }
    }


    for (int i = 0; i < DATA_SIZE; ++i)
    {
        std::vector<uint8_t> resultVector;
        unsigned resultVersion;

        bool ok = Base58Check::decode(DATA[i].stringVersion, resultVector, resultVersion);
        if (ok &&
            resultVector.size() == DATA[i].size &&
            std::equal(resultVector.begin(), resultVector.end(), DATA[i].hash) &&
            resultVersion == DATA[i].version)
        {
            printf("decode %d: ok\n", i);
        }
        else
        {
            printf("decode %d: error\n", i);
        }
    }
}