#include "../shared.h"
#include "../targetver.h"
#include "CppUnitTest.h"

#include "utility/Utility.h"
#include <cstdio>

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestCrypto
{

TEST_CLASS(Crypto_Random)
{
public:
    TEST_METHOD(Crypto_Random_addEntropy)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Random_getBytes)
    {
        Assert::Fail(L"Not implemented");
    }
};

} // namespace TestCrypto
