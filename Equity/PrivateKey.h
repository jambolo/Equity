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
        PrivateKey(std::vector<uint8_t> const & k, unsigned v);
        PrivateKey(uint8_t const * k, unsigned v);

        std::vector<uint8_t> value() const  { return value_; }
        bool valid() const                  { return valid_; }

        std::string toWif() const;
        std::string toHex() const;

    private:

        bool isValid();

        std::vector<uint8_t> value_;
        unsigned version_;
        bool valid_;
    };

}