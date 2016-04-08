#include "../targetver.h"
#include "CppUnitTest.h"

#include "equity/PrivateKey.h"
#include "utility/Utility.h"

#include <string>
#include <vector>
#include <cstdio>

using namespace Equity;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace TestEquity
{

    static unsigned const BITCOIN_PRIVATE_KEY_VERSION = 0x80;

    struct WifInput
    {
        uint8_t data[32];
        char const * wif;
        char const * wifCompressed;
        bool valid;
    };

    static WifInput WIF_CASES[] =
    {
        {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1 },
            "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAnchuDf",
            "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73sVHnoWn",
            true
        },
        {
            {
                0x6E, 0x12, 0x0E, 0xA6, 0x23, 0xBC, 0xAB, 0x6E, 0x77, 0xD4, 0x63, 0x0B, 0x44, 0x2E, 0x3D, 0x98,
                0x3F, 0xAE, 0x17, 0xF1, 0x7D, 0xA6, 0xF0, 0x59, 0x20, 0x3A, 0x4C, 0x37, 0x1E, 0xD3, 0x29, 0x8E
            },
            "5JemBigghFJRCydhheaeNbeHqDnYmpxJMiNEsyMUyyZmAfpwoUM",
            "Kzug2JzACe7sqrxzeQsVxKubHqS3Erf5cRakFyjQWUfbwbm1hBUS",
            true
        },
        {
            {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
                0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x40
            },
            "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetqj84qw",
            "L5oLkpV3aqBjhki6LmvChTCV6odsp4SXM6FfU2Gppt5kFLaHLuZ9",
            true
        },
        {
            {
                0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE,
                0xBA, 0xAE, 0xDC, 0xE6, 0xAF, 0x48, 0xA0, 0x3B, 0xBF, 0xD2, 0x5E, 0x8C, 0xD0, 0x36, 0x41, 0x41 // 1 more than max
            },
            "5Km2kuu7vtFDPpxywn4u3NLpbr5jKpTB3jsuDU2KYEqetwr388P",
            "L5oLkpV3aqBjhki6LmvChTCV6odsp4SXM6FfU2Gppt5kFqRzExJJ",
            false
        },
        {
            { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }, // 0 is invalid
            "5HpHagT65TZzG1PH3CSu63k8DbpvD8s5ip4nEB3kEsreAbuatmU",
            "KwDiBf89QgGbjEhKnhXJuH7LrciVrZi3qYjgd9M7rFU73Nd2Mcv1",
            false
        }
    };

    struct WifValidityInput
    {
        char const * wif;
        bool expected;
    };

    static WifValidityInput const WIF_VALIDITY_CASES[] =
    {
        { "", false },
        { "26k9aD1PF", false },                                             // minimum base58check, but too short
        { "yg26Xdu6SVmmVddThR8kwpf49TZ8UT9WDTLbM1FpLPzvDafte", false },     // one byte too short
        { "L5nzLB4U8X8qTSALobGXntqTHfpbZucCb46z6NUChsChwAXRV5JP", false },  // invalid compressed key, flag == 0xff instead of 0x01
        { "51JtGrFPM4GihbattjeNy4rXeL7GGhurMeqbubbepp48jUGJfj8G", false },  // one byte too big
        { "5K9DqQDrw1XzbPNfvqPXExyLxtLCozAQXwyFUbJ8PvcWPMx5bja", false },   // checksum is wrong
        { "Kzug2JzACe7sqrxzeQsVxKubHqS3Erf5cRakFyjQWUfbwbm1hBUT", false }   // checksum is wrong
    };

    struct MiniInput
    {
        char const * mini;
        uint8_t data[32];
        bool valid;
    };

    static MiniInput MINI_CASES[] =
    {
        {
            "S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy",
            {
                0x4C, 0x7A, 0x96, 0x40, 0xC7, 0x2D, 0xC2, 0x09, 0x9F, 0x23, 0x71, 0x5D, 0x0C, 0x8A, 0x0D, 0x8A,
                0x35, 0xF8, 0x90, 0x6E, 0x3C, 0xAB, 0x61, 0xDD, 0x3F, 0x78, 0xB6, 0x7B, 0xF8, 0x87, 0xC9, 0xAB
            },
            true
        },
        {
            "SpGW5Jtdt4E4rcK1LJjzqC35LJWpJU",
            {
                0x6D, 0x9F, 0x30, 0x05, 0xB8, 0xD4, 0x42, 0x2C, 0x32, 0x2D, 0x93, 0x82, 0x9F, 0x6A, 0xEC, 0x9F,
                0x69, 0x2E, 0x6D, 0x04, 0x5C, 0x0E, 0xED, 0x3C, 0xDA, 0x56, 0xA1, 0xA0, 0xC3, 0xDA, 0x03, 0x56
            },
            true,
        },
        {
            "SSSSSSSSSSSSSSSSSSSSSSSSSSSSSS",   // checksum is not 0
            { },
            false,
        },
        {
            "TpGW5Jtdt4E4rcK1LJjzqC35LJWpJU",   // doesn't start with 'S'
            { },
            false,
        },

    };

    TEST_CLASS(Equity_PrivateKeyTest)
    {
    public:

        TEST_METHOD(Equity_PrivateKey_constuctor_uint8_t_ptr)
        {
            for (auto const & c : WIF_CASES)
            {
                PrivateKey result(c.data);

                Assert::AreEqual(c.valid, result.valid());                                                  // Validity should match
                if (result.valid())
                {
                    std::array<uint8_t, PrivateKey::SIZE> value = result.value();
                    Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));                         // Value should match
                    Assert::IsFalse(result.compressed());                                                   // Should not be compressed
                    Assert::AreEqual(Utility::toHex(c.data, sizeof(c.data)), result.toHex());               // Hex should match
                    Assert::AreEqual(c.wif, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());             // WIF should match
                    result.setCompressed(true);
                    Assert::IsTrue(result.compressed());                                                    // Should be compressed now
                    Assert::AreEqual(c.wifCompressed, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());   // Compressed WIF should match
                    result.setCompressed(false);
                    Assert::IsFalse(result.compressed());                                                   // Should not be compressed now
                    Assert::AreEqual(c.wif, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());             // WIF should match
                    result.setCompressed();
                    Assert::IsTrue(result.compressed());                                                    // Should be compressed now
                    Assert::AreEqual(c.wifCompressed, result.toWif(BITCOIN_PRIVATE_KEY_VERSION).c_str());   // WIF should match
                }
            }
        }

        TEST_METHOD(Equity_PrivateKey_constuctor_vector)
        {
            for (auto const & c : WIF_CASES)
            {
                PrivateKey result(std::vector<uint8_t>(&c.data[0], &c.data[sizeof(c.data)]));

                Assert::AreEqual(c.valid, result.valid());                                                  // Validity should match
                if (result.valid())
                {
                    std::array<uint8_t, PrivateKey::SIZE> value = result.value();
                    Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));                         // Value should match
                    Assert::IsFalse(result.compressed());                                                   // Should not be compressed
                    Assert::AreEqual(Utility::toHex(c.data, sizeof(c.data)), result.toHex());               // Hex should match
                }
            }

            auto const & c0 = WIF_CASES[0];
            std::vector<uint8_t> invalidInput(&c0.data[0], &c0.data[sizeof(c0.data)]);

            invalidInput.resize(PrivateKey::SIZE - 1);  // One byte too small
            PrivateKey result1(invalidInput);
            Assert::IsFalse(result1.valid());           // Should not be valid

            invalidInput.resize(PrivateKey::SIZE + 1);  // One byte too big
            PrivateKey result2(invalidInput);
            Assert::IsFalse(result2.valid());           // Should not be valid
        }

        TEST_METHOD(Equity_PrivateKey_constuctor_string)
        {
            for (auto const & c : WIF_CASES)
            {
                {
                    PrivateKey result(std::string(c.wif));

                    Assert::AreEqual(c.valid, result.valid());                              // Validity should match
                    if (result.valid())
                    {
                        std::array<uint8_t, PrivateKey::SIZE> value = result.value();
                        Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));     // Value should match
                        Assert::IsFalse(result.compressed());                               // Should not be compressed
                    }
                }
                {
                    PrivateKey result(std::string(c.wifCompressed));

                    Assert::AreEqual(c.valid, result.valid());
                    if (result.valid())
                    {
                        std::array<uint8_t, PrivateKey::SIZE> value = result.value();
                        Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));
                        Assert::IsTrue(result.compressed());
                    }
                }
            }

            for (auto const & c : WIF_VALIDITY_CASES)
            {
                PrivateKey result(std::string(c.wif));
                Assert::AreEqual(c.expected, result.valid());
            }

            for (auto const & c : MINI_CASES)
            {
                PrivateKey result(std::string(c.mini));

                Assert::AreEqual(c.valid, result.valid());                              // Validity should match
                if (result.valid())
                {
                    std::array<uint8_t, PrivateKey::SIZE> value = result.value();
                    Assert::IsTrue(std::equal(value.begin(), value.end(), c.data));     // Value should match
                    Assert::IsFalse(result.compressed());                               // Should not be compressed
                }
            }
        }
    };

} // namespace TestEquity
