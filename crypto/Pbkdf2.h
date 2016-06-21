#pragma once

#include <vector>

namespace Crypto
{

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

} // namespace Crypto
