#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class PrivateKey
    {
    public:

        static size_t const SIZE = 256 / 8;
        static int const COMPRESSED_FLAG = 0x1;

        explicit PrivateKey(std::vector<uint8_t> const & k);
        explicit PrivateKey(uint8_t const * k);
        explicit PrivateKey(std::string const & wif);

        void setCompressed(bool yes = true) { compressed_ = yes; }

        std::vector<uint8_t> value() const  { return value_; }
        bool compressed() const             { return compressed_; }
        bool valid() const                  { return valid_; }

        std::string toWif(unsigned v) const;
        std::string toHex() const;

    private:

        bool isValid();

        std::vector<uint8_t> value_;
        bool valid_;
        bool compressed_;
    };

}