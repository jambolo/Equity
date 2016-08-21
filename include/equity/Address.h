#pragma once

#include "crypto/Ripemd.h"
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

class PublicKey;

//! A Bitcoin address

class Address
{
public:

    //! Number of bytes in the unencoded binary form
    static size_t const SIZE = Crypto::RIPEMD160_HASH_SIZE;

    // Constructor
    //!
    //! @param  s   address in Base58Check form
    explicit Address(std::string const & s);

    // Constructor
    //!
    //! @param  data    start of address in binary form
    //! @param  size    size of the data
    Address(uint8_t const * data, size_t size);

    // Constructor
    //!
    //! @param  k   address in binary form
    explicit Address(Crypto::Ripemd160Hash const & k);

    // Constructor
    //!
    //! @param  publicKey   public key used to generate the address
    explicit Address(PublicKey const & publicKey);

    //! Returns the address in binary form
    std::array<uint8_t, SIZE> value() const { return value_; }

    //! Returns true if the address is valid
    bool valid() const { return valid_; }

    //! Returns the address in Base58Check form
    std::string toString(unsigned network) const;

private:

    std::array<uint8_t, SIZE> value_;
    bool valid_;
};

} // namespace Equity
