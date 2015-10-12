#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include "PrivateKey.h"

namespace Equity {

    class PublicKey
    {
    public:

        static size_t const SIZE = 512 / 8;

        explicit PublicKey(std::vector<uint8_t> const & k);
        explicit PublicKey(uint8_t const * k);
        explicit PublicKey(PrivateKey const & k);

        std::vector<uint8_t> value() const  { return value_; }
        bool valid() const                  { return valid_; }

        std::string toHex() const;

    private:

        std::vector<uint8_t> value_;
        bool valid_;
    };

}