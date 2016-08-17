#pragma once
#if !defined(UNIT_TEST_SHARED_H_INCLUDED)
  #define UNIT_TEST_SHARED_H_INCLUDED

  #include <string>

std::wstring hashErrorMessage(wchar_t const * test,
                              char const *    input,
                              uint8_t const * expected,
                              size_t          expectedSize,
                              uint8_t const * actual,
                              size_t          actualSize);

std::wstring pbkdf2ErrorMessage(wchar_t const * test,
                                char const *    password,
                                char const *    salt,
                                int             count,
                                uint8_t const * expected,
                                size_t          expectedSize,
                                uint8_t const * actual,
                                size_t          actualSize);

std::wstring hmacErrorMessage(wchar_t const * test,
                              char const *    key,
                              char const *    data,
                              uint8_t const * expected,
                              size_t          expectedSize,
                              uint8_t const * actual,
                              size_t          actualSize);

#endif // !defined(UNIT_TEST_SHARED_H_INCLUDED)
