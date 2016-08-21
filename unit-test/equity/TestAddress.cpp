#include "../targetver.h"
#include "CppUnitTest.h"

#include "equity/Address.h"
#include "equity/PublicKey.h"
#include "utility/Utility.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace Equity;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestEquity
{

static int const MAIN_NETWORK_ID = 0;
static int const TEST_NETWORK_ID = 0xc4;
static int const P2SH_NETWORK_ID = 5;

struct TestInput
{
    int version;
    uint8_t data[20];
    char const * base58Check;
};

static TestInput const CASES[] =
{
    { MAIN_NETWORK_ID, { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, "1111111111111111111114oLvT2" },
    { MAIN_NETWORK_ID,
      { 0x01, 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B,
        0xEE }, "16UwLL9Risc3QfPqBUvKofHmBQ7wMtjvM" },
    { MAIN_NETWORK_ID,
      { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
        0xFF }, "1QLbz7JHiBTspS962RLKV8GndWFwi5j6Qr" },
    { TEST_NETWORK_ID,
      { 0x09, 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE,
        0x01 }, "2Mt6vpVdNZziX6xefS5BWZQ5PjGH9cabz7v" },
    { P2SH_NETWORK_ID,
      { 0x66, 0x77, 0x60, 0x06, 0x95, 0x3D, 0x55, 0x67, 0x43, 0x9E, 0x5E, 0x39, 0xF8, 0x6A, 0x0D, 0x27, 0x3B, 0xEE, 0x01,
        0x09 }, "3B2orSMAV7jhCgubX9NphAA2uXkFkeJ61y" },
};

struct StringValidityInput
{
    char const * input;
    bool expected;
};
static StringValidityInput const STRING_VALIDITY_CASES[] =
{
    { "", false },
    { "112edB6q", false },                              // minimum base58check, but too short
    { "137kV53f3mW571W38VFn9WBSmrx1eiMNy", false },     // one byte too short
    { "1iEMistU6HpJwnLgUixyZYvURF28JGhxvSx", false },   // one byte too big
    { "1BadxxxxxxxxxxxxxxxxxxxxxxxvvKy2F", false }      // invalid checksum
};

struct PublicKeyInput
{
    uint8_t publicKey[PublicKey::UNCOMPRESSED_SIZE];
    char const * address;
};

static PublicKeyInput PUBLIC_KEY_CASES[] =
{
    { { 0x04,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
      "16QaFeudRUt8NYy2yzjm3BMvG4xBbAsBFM" },
    { { 0x04,
        0x78, 0xD4, 0x30, 0x27, 0x4F, 0x8C, 0x5E, 0xC1, 0x32, 0x13, 0x38, 0x15, 0x1E, 0x9F, 0x27, 0xF4,
        0xC6, 0x76, 0xA0, 0x08, 0xBD, 0xF8, 0x63, 0x8D, 0x07, 0xC0, 0xB6, 0xBE, 0x9A, 0xB3, 0x5C, 0x71,
        0xA1, 0x51, 0x80, 0x63, 0x24, 0x3A, 0xCD, 0x4D, 0xFE, 0x96, 0xB6, 0x6E, 0x3F, 0x2E, 0xC8, 0x01,
        0x3C, 0x8E, 0x07, 0x2C, 0xD0, 0x9B, 0x38, 0x34, 0xA1, 0x9F, 0x81, 0xF6, 0x59, 0xCC, 0x34, 0x55 },
      "1JwSSubhmg6iPtRjtyqhUYYH7bZg3Lfy1T" }
};

TEST_CLASS(Equity_AddressTest)
{
public:

    TEST_METHOD(Equity_Address_constuctor_string)
    {
        for (auto const & c : STRING_VALIDITY_CASES)
        {
            Address result(c.input);
            Assert::AreEqual(c.expected, result.valid());
        }
        for (auto const & c : CASES)
        {
            Address result(c.base58Check);
            Assert::IsTrue(result.valid());
            if (result.valid())
            {
                std::array<uint8_t, Address::SIZE> value = result.value();
                Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));
            }
        }
    }

    TEST_METHOD(Equity_Address_constuctor_uint8_t_ptr)
    {
        for (auto const & c : CASES)
        {
            Address result(c.data, sizeof(c.data));
            Assert::IsTrue(result.valid());
            if (result.valid())
            {
                std::array<uint8_t, Address::SIZE> value = result.value();
                Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));
                Assert::AreEqual(c.base58Check, result.toString(c.version).c_str());
            }
        }
    }

    TEST_METHOD(Equity_Address_constuctor_Ripemd160Hash)
    {
        for (auto const & c : CASES)
        {
            Crypto::Ripemd160Hash hash;
            std::copy(c.data, c.data + sizeof(c.data), hash.data());
            Address result(hash);
            Assert::IsTrue(result.valid());
            if (result.valid())
            {
                std::array<uint8_t, Address::SIZE> value = result.value();
                Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));
                Assert::AreEqual(c.base58Check, result.toString(c.version).c_str());
            }
        }
    }

    TEST_METHOD(Equity_Address_constuctor_publicKey)
    {
        for (auto const & c : PUBLIC_KEY_CASES)
        {
            Address result(PublicKey(c.publicKey, sizeof(c.publicKey)));
            Assert::IsTrue(result.valid());
            if (result.valid())
            {
                std::array<uint8_t, Address::SIZE> value = result.value();
                Assert::AreEqual(c.address, result.toString(MAIN_NETWORK_ID).c_str());
            }
        }
    }
};

} // namespace TestEquity
