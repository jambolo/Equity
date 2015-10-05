#include "equity/Base5.h"
#include "Base58Check.h"


void TestBase58Check();

int _tmain(int argc, _TCHAR* argv[])
{
    // Test Base58Check
    testBase58Check();

	return 0;
}

void TestBase58Check()
{
    uint8_t const tests[][20] =
    {
        { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE },
        { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF }
    };
    char const * const expected[] =
    {
        "1111111111111111111114oLvT2",
        "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM",
        "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr"
    };

    Base58Check base58Check;

    for (int i = 0; i < 3; ++i)
    {
        std::string result = base58Check(tests[i], 20);
        if (result != expectResult[i])
            printf("error\n");
    }
}