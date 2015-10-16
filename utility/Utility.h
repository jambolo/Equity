#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <crypto/Sha256.h>

namespace Utility
{

    //! Converts a vector to a hex string
    std::string vtox(std::vector<uint8_t> const & v);

    //! Converts a vector to a hex string
    std::string vtox(uint8_t const * v, size_t length);

    //! Converts a hex string to a vector
    std::vector<uint8_t> xtov(std::string const & x);

    //! Converts a hex string to a vector
    std::vector<uint8_t> xtov(char const * x, size_t length);


} // namespace Utility
