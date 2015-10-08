#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class Base58
    {
    public:
        std::string operator ()(std::vector<uint8_t> const & input);
        std::string operator ()(uint8_t const * input, size_t length);

        bool decode(std::string const & input, std::vector<uint8_t> & output);
        bool decode(char const * input, size_t length, std::vector<uint8_t> & output);
    };

}