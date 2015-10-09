#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Equity {

    namespace Base58Check
    {
        std::string encode(std::vector<uint8_t> const & input, unsigned version);
        std::string encode(uint8_t const * input, size_t length, unsigned version);
        bool decode(std::string const & input, std::vector<uint8_t> & output, unsigned & version);
    }

}