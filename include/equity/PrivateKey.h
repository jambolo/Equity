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
    PrivateKey(uint8_t const * begin, uint8_t const * end);

    // Constructor
    explicit PrivateKey(std::vector<uint8_t> const & k);

    // Constructor
    explicit PrivateKey(std::string const & wif);

    //! Sets the compressed flag
    void setCompressed(bool yes = true) { compressed_ = yes; }

    //! Returns the value as an array of bytes
    std::array<uint8_t, SIZE> value() const { return value_; }

    //! Returns the value of the compressed flag
    bool compressed() const { return compressed_; }

    //! Returns true if this private key is valid
    bool valid() const { return valid_; }

    //! Returns a WIF format string
    std::string toWif(unsigned v) const;

    //! Returns a hex format string
    std::string toHex() const;

private:

    static int const COMPRESSED_FLAG = 0x1;

    bool isValid();

    std::array<uint8_t, SIZE> value_;
    bool valid_;
    bool compressed_;
};

} // namespace Equity
