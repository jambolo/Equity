#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class PrivateKey
    {
    public:

        static size_t const SIZE = 256 / 8;

        explicit PrivateKey(std::string const & wif);
        explicit PrivateKey(std::vector<uint8_t> const & k);
        explicit PrivateKey(uint8_t const * k);

        std::vector<uint8_t> value() const  { return value_; }
        bool valid() const                  { return valid_; }

        std::string toWif(unsigned v) const;
        std::string toHex() const;

    private:

        bool isValid();

        std::vector<uint8_t> value_;
        unsigned version_;
        bool valid_;
    };

}