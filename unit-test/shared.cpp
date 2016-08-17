#include "shared.h"

#include "utility/Utility.h"
#include <sstream>

#include "CppUnitTest.h"

using namespace Crypto;
using namespace Utility;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

std::wstring hashErrorMessage(wchar_t const * test,
                              char const *    input,
                              uint8_t const * expected,
                              size_t          expectedSize,
                              uint8_t const * actual,
                              size_t          actualSize)
{
    std::wostringstream message;
    message
        << test
        << L"(\""
        << ToString(shorten(input))
        << L"\"): expected "
        << ToString(toHex(expected, expectedSize))
        << ", got "
        << ToString(toHex(actual, actualSize));
    return message.str();
}

std::wstring pbkdf2ErrorMessage(wchar_t const * test,
                                char const *    password,
                                char const *    salt,
                                int             count,
                                uint8_t const * expected,
                                size_t          expectedSize,
                                uint8_t const * actual,
                                size_t          actualSize)
{
    std::wostringstream message;
    message
        << test
        << L"(\""
        << ToString(shorten(password))
        << L"\", \""
        << ToString(shorten(salt))
        << L"\""
        << count
        << "): expected "
        << ToString(toHex(expected, expectedSize))
        << ", got "
        << ToString(toHex(actual, actualSize));
    return message.str();
}

std::wstring hmacErrorMessage(wchar_t const * test,
                              char const *    key,
                              char const *    data,
                              uint8_t const * expected,
                              size_t          expectedSize,
                              uint8_t const * actual,
                              size_t          actualSize)
{
    std::wostringstream message;
    message
        << test
        << L"(\""
        << ToString(shorten(key))
        << L"\", \""
        << ToString(shorten(data))
        << L"\"): expected "
        << ToString(toHex(expected, expectedSize))
        << ", got "
        << ToString(toHex(actual, actualSize));
    return message.str();
}
