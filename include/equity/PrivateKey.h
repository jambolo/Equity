#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

class PrivateKey
{
public:
    //! Number of bytes in a private key
    static size_t const SIZE = 256 / 8;

    // Constructor
    //!
    //! The object is constructed from the bytes in the range [begin, end). If the size of the range is not 
    //! SIZE bytes or the value is not valid, the constructed object is marked as invalid.
    //!
    //! @param  data    pointer to start of the data
    //! @param  size    length of the data
    PrivateKey(uint8_t const * data, size_t size);

    // Constructor
    //!
    //! The object is constructed from the bytes in the given vector. If the size of the vector is not SIZE or the
    //! value is not valid, the constructed object is marked as invalid.
    //!
    //! @param  k   vector of bytes
    explicit PrivateKey(std::vector<uint8_t> const & k);

    // Constructor
    //!
    //! The input can be in either the WIF format or the mini-key format. If the format is invalid or the value of the
    //! key is invalid, the contructed object is marked as invalid.
    //!
    //! @param  s   string form of the private key
    explicit PrivateKey(std::string const & s);

    //! Sets the compressed flag
    void setCompressed(bool yes = true) { compressed_ = yes; }

    //! Returns the value as an array of bytes.
    std::array<uint8_t, SIZE> value() const { return value_; }

    //! Returns the value of the compressed flag.
    bool compressed() const { return compressed_; }

    //! Returns true if this private key is valid
    bool valid() const { return valid_; }

    //! Returns a WIF format string.
    //!
    //! @param  version     version byte to be prepended (typically 0x80)
    //!
    //! @return     a string containing the private key in WIF format, or empty if the object is invalid
    std::string toWif(unsigned v) const;

    //! Returns a hex format string.
    //!
    //! @return     a string containing the private key in hex format, or empty if the object is invalid
    std::string toHex() const;

private:

    static int const COMPRESSED_FLAG = 0x1;

    bool isValid();

    std::array<uint8_t, SIZE> value_;
    bool valid_;
    bool compressed_;
};

} // namespace Equity
