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
        BitcoinAddress(Crypto::Ripemd::Hash const & k, unsigned network);

        std::string toString() const            { return string_; }
        std::vector<uint8_t> toBytes() const    { return data_; }
        int version() const                     { return version_; }

    private:

        std::string string_;
        std::vector<uint8_t> data_;
        unsigned version_;
    };

}