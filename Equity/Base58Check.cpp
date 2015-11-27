#include "Base58Check.h"

#include "Base58.h"
#include "crypto/Sha256.h"

#include <algorithm>

using namespace Equity;
using namespace Crypto;

std::string Base58Check::encode(std::vector<uint8_t> const & input, unsigned version)
{
    return encode(input.data(), input.size(), version);
}

std::string Base58Check::encode(uint8_t const * input, size_t length, unsigned version)
{
    // 1. Take the version byte and payload bytes, and concatenate them together(bytewise).
    std::vector<uint8_t> work;
    work.push_back(version);
    work.insert(work.end(), input, input + length);

    // 2. Take the first four bytes of SHA256(SHA256(results of step 1))
    std::vector<uint8_t> check = checksum(work);

    // 3. Concatenate the results of step 1 and the results of step 2 together(bytewise).
    work.insert(work.end(), check.begin(), check.end());

    // 4. Treating the results of step 3 - a series of bytes - as a single big - endian bignumber, convert to
    //    base-58 ... The result should be normalized to not have any leading base-58 zeroes(character '1').

    std::string workString = Base58::encode(work);

    // 5. The leading character '1', which has a value of zero in base58, is reserved for representing an entire
    //    leading zero byte, as when it is in a leading position, has no value as a base - 58 symbol. There can be
    //    one or more leading '1's when necessary to represent one or more leading zero bytes. Count the number of
    //    leading zero bytes that were the result of step 3 (for old Bitcoin addresses, there will always be at
    //    least one for the version / application byte; for new addresses, there will never be any). Each leading
    //    zero byte shall be represented by its own character '1' in the final result.

    int leadingZeros = 0;
    for (auto i: work)
    {
        if (i != 0)
        {
            break;
        }
        ++leadingZeros;
    }

    // 6. Concatenate the 1's from step 5 with the results of step 4. This is the Base58Check result.

    std::string output(leadingZeros, '1');
    output += workString;

    return output;
}

bool Base58Check::decode(std::string const & input, std::vector<uint8_t> & output, unsigned & version)
{
    return decode(input.c_str(), output, version);
}

bool Base58Check::decode(char const * input, std::vector<uint8_t> & output, unsigned & version)
{
    // Skip and count the leading 1's
    int nLeadingOnes = 0;
    while (*input == '1')
    {
        ++nLeadingOnes;
        ++input;
    }

    // Decode the input into binary
    std::vector<uint8_t> work;
    if (!Base58::decode(input, work))
    {
        return false;
    }

    // Prepend the leading zeros
    work.insert(work.begin(), nLeadingOnes, 0);

    // Make sure there is a 1 byte version, 4 byte checksum, and at least one byte of data
    if (work.size() < 6)
    {
        return false;
    }

    // Check the checksum
    std::vector<uint8_t> check = doubleSha256(work.data(), work.size() - 4);
    if (!std::equal(check.begin(), check.begin() + 4, work.end() - 4))
    {
        return false;
    }

    version = work[0];
    output.assign(work.begin() + 1, work.end() - 4);
    return true;
}
