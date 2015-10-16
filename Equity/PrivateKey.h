#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class PrivateKey
    {
    public:

        static size_t const SIZE = 256 / 8;

        explicit PrivateKey(std::vector<uint8_t> const & k);
        explicit PrivateKey(uint8_t const * k);
        explicit PrivateKey(std::string const & wif);

        std::vector<uint8_t> value() const  { return value_; }
        bool valid() const                  { return valid_; }

        std::string toWif(unsigned v) const;
        std::string toHex() const;

    private:

        bool isValid();

        std::vector<uint8_t> value_;
        bool valid_;
    };

}