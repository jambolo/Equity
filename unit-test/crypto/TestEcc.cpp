#include "../shared.h"
#include "../targetver.h"
#include "CppUnitTest.h"

#include "crypto/Ecc.h"
#include "utility/Utility.h"
#include <cstdio>

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestCrypto
{

TEST_CLASS(Crypto_Ecc)
{
public:
    TEST_METHOD(Crypto_Ecc_publicKeyIsValid_ptr_size)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_publicKeyIsValid_PublicKey)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_privateKeyIsValid_ptr_size)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_privateKeyIsValid_PrivateKey)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_derivePublicKey_PrivateKey)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_sign)
    {
        Assert::Fail(L"Not implemented");
    }

    TEST_METHOD(Crypto_Ecc_verify)
    {
        Assert::Fail(L"Not implemented");
    }
};

} // namespace TestCrypto
