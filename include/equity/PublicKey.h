#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

class PrivateKey;

//! A Bitcoin public key.
//!
//! Bitcoin public keys are assumed to be in the form specified by ANSI X9.62.

class PublicKey
{
public:

    static size_t const COMPRESSED_SIZE = 33;   //!< Size of a compressed public key
    static size_t const UNCOMPRESSED_SIZE = 65; //!< Size of an uncompressed public key

    // Constructor
    //!
    //! @param  k       raw key bytes
    explicit PublicKey(std::vector<uint8_t> const & k);

    // Constructor
    //!
    //! @param  data    raw key bytes
    //! @param  size    size of the data
    PublicKey(uint8_t const * data, size_t size);

    // Constructor
    //!
    //! @param  k       private key
    explicit PublicKey(PrivateKey const & k);

    //! Returns the raw key bytes
    std::vector<uint8_t> value() const { return value_; }

    //! Returns true if the public key is compressed
    bool compressed() const { return compressed_; }

    //! Returns true if the key is valid
    bool valid() const { return valid_; }

    //! Returns the key as a hex string
    std::string toHex() const;

private:

    std::vector<uint8_t> value_;
    bool valid_;
    bool compressed_;
};

} // namespace Equity
