#pragma once

#include "crypto/Ripemd.h"
#include <string>
#include <vector>
#include <cstdint>

namespace Equity {

    class PublicKey;
    class PrivateKey;

    class Address
    {
    public:

        static size_t const SIZE = Crypto::RIPEMD160_HASH_SIZE;

        explicit Address(std::string const & s);
        explicit Address(Crypto::Ripemd160Hash const & k);
        explicit Address(uint8_t const * k);
        explicit Address(PublicKey const & publicKey);
        explicit Address(PrivateKey const & privateKey);

        std::vector<uint8_t> value() const      { return value_; }
        bool valid() const                      { return valid_; }

        std::string toString(unsigned network) const;

    private:

        std::vector<uint8_t> value_;
        bool valid_;
    };

}