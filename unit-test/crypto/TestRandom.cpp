#include "../shared.h"
#include "../targetver.h"
#include "CppUnitTest.h"

#include "crypto/Random.h"
#include "utility/Utility.h"
#include <cstdio>

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

static size_t const SIZE = 256;
static size_t const SAFE_SIZE = SIZE / 2 - 1;   // Make it odd to catch alignment assumptions

static uint8_t const ZEROS[SIZE];

namespace TestCrypto
{

TEST_CLASS(Crypto_Random)
{
public:
    TEST_METHOD(Crypto_Random_status)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Random_addEntropy)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Random_getBytes)
    {

        uint8_t buffer1[SIZE];
        std::fill(buffer1, buffer1 + sizeof(buffer1), 0);

        Random::getBytes(buffer1, SAFE_SIZE);

        // Ensure that too many bytes were not generated
        Assert::IsTrue(std::equal(buffer1 + SAFE_SIZE, buffer1 + sizeof(buffer1), ZEROS), L"Generated too many bytes");

        uint8_t buffer2[SIZE];
        std::fill(buffer2, buffer2 + sizeof(buffer2), 0);

        Random::getBytes(buffer2, SAFE_SIZE);

        // Ensure that the bytes vary between calls
        Assert::IsFalse(std::equal(buffer1, buffer1 + SAFE_SIZE, buffer2), L"Generated the same bytes as the last call");

        // Ensure that too few bytes were not generated
        Assert::IsTrue(buffer1[SAFE_SIZE - 1] != 0 || buffer2[SAFE_SIZE - 1] != 0, L"Generated too few bytes");
    }
};

} // namespace TestCrypto
