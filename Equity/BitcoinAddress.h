#pragma once

#include "crypto/Ripemd.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class BitcoinAddress
    {
    public:
        BitcoinAddress(std::string const & s);
        BitcoinAddress(Crypto::Ripemd160Hash const & k, unsigned n);

        std::string toString() const            { return string_; }
        std::vector<uint8_t> toBytes() const    { return data_; }
        int network() const                     { return network_; }
        bool valid() const                      { return valid_; }

    private:

        std::vector<uint8_t> data_;
        unsigned network_;
        std::string string_;
        bool valid_;
    };

}