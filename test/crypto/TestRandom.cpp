#include "crypto/Random.h"
#include "utility/Utility.h"

#include <gtest/gtest.h>

using namespace Crypto;

namespace
{
    static size_t const SIZE      = 256;
    static size_t const SAFE_SIZE = SIZE / 2 - 1;   // Make it odd to catch alignment assumptions

    static uint8_t const ZEROS[SIZE];
} // anonymous namespace

TEST(CryptoRandomTest, status)
{
    GTEST_SKIP();
}

TEST(CryptoRandomTest, addEntropy)
{
    GTEST_SKIP();
}

TEST(CryptoRandomTest, getBytes)
{
    uint8_t buffer1[SIZE];
    std::fill(buffer1, buffer1 + sizeof(buffer1), 0);

    Random::getBytes(buffer1, SAFE_SIZE);

    // Ensure that too many bytes were not generated
    EXPECT_TRUE(std::equal(buffer1 + SAFE_SIZE, buffer1 + sizeof(buffer1), ZEROS));

    uint8_t buffer2[SIZE];
    std::fill(buffer2, buffer2 + sizeof(buffer2), 0);

    Random::getBytes(buffer2, SAFE_SIZE);

    // Ensure that the bytes vary between calls
    EXPECT_FALSE(std::equal(buffer1, buffer1 + SAFE_SIZE, buffer2));

    // Ensure that too few bytes were not generated
    EXPECT_TRUE(buffer1[SAFE_SIZE - 1] != 0 || buffer2[SAFE_SIZE - 1] != 0);
}

int main(int argc, char ** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
