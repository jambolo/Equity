#pragma once

#include <vector>
#include <cstdint>

namespace Crypto
{
//! @addtogroup CryptoGroup
//!@{

//! Returns the result of a PBKDF2 operation using SHA-512.
//!
//! @param  password    password
//! @param  salt        salt
//! @param  count       number of iterations
//! @param  size        desired size of the returned value
std::vector<uint8_t> pbkdf2HmacSha512(std::vector<uint8_t> const & password,
                                      std::vector<uint8_t> const & salt,
                                      int                          count,
                                      size_t                       size);

//! Returns the result of a PBKDF2 operation using SHA-512.
//!
//! @param  password        password
//! @param  passwordSize    length of the password
//! @param  salt            salt
//! @param  saltSize        length of the salt
//! @param  count           number of iterations
//! @param  size            desired size of the returned value
std::vector<uint8_t> pbkdf2HmacSha512(uint8_t const * password,
                                      size_t          passwordSize,
                                      uint8_t const * salt,
                                      size_t          saltSize,
                                      int             count,
                                      size_t          size);

//!@}

/********************************************************************************************************************/

inline std::vector<uint8_t> pbkdf2HmacSha512(std::vector<uint8_t> const & password,
                                             std::vector<uint8_t> const & salt,
                                             int                          count,
                                             size_t                       size)
{
    return pbkdf2HmacSha512(password.data(), password.size(), salt.data(), salt.size(), count, size);
}
} // namespace Crypto
