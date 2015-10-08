#pragma once

#include <string>
#include <cstdint>
#include <vector>

namespace Equity {

    class Base58Check
    {
    public:
        std::string operator ()(std::vector<uint8_t> const & input, unsigned version);
        std::string operator ()(uint8_t const * input, size_t length, unsigned version);

        bool decode(std::string const & input, std::vector<uint8_t> & output, unsigned & version);
    };

}