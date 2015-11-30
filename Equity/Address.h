#pragma once

#include "crypto/Ripemd.h"
#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace Equity
{

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

    std::array<uint8_t, SIZE> value() const { return value_; }
    bool valid() const { return valid_; }

    std::string toString(unsigned network) const;

private:

    std::array<uint8_t, SIZE> value_;
    bool valid_;
};

} // namespace Equity
