#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace Equity
{

class PrivateKey;

class PublicKey
{
public:

    static size_t const SIZE = 512 / 8 + 1;

    explicit PublicKey(std::vector<uint8_t> const & k);
    explicit PublicKey(uint8_t const * k);
    explicit PublicKey(PrivateKey const & k);

    std::array<uint8_t, SIZE> value() const { return value_; }
    bool valid() const { return valid_; }

    std::string toHex() const;

private:

    std::array<uint8_t, SIZE> value_;
    bool valid_;
};

} // namespace Equity
