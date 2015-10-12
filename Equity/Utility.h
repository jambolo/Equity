#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Utility
{

    std::string vtox(std::vector<uint8_t> const & v);
    std::string vtox(uint8_t const * v, size_t length);
    std::vector<uint8_t> xtov(std::string const & x);
    std::vector<uint8_t> xtov(char const * x, size_t length);

}

