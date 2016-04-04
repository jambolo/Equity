#pragma once

#include "crypto/Sha256.h"
#include <cstdint>
#include <vector>
#include "p2p/Serialize.h"

namespace Network
{

class BitArray
{
public:

    // Constructor
    BitArray() {}

    // Constructor
    BitArray(size_t size)
        : bits_((size + 7) / 8, 0)
    {
    }

    // Constructor
    BitArray(uint8_t const * & in, size_t & size)
    {
        bits_ = P2p::VarArray<uint8_t>(in, size).value();
    }

    //! Returns the value of the bit at the given index
    bool get(size_t index) const
    {
        size_t i = index / 8;
        if (i >= bits_.size())
            return false;
        size_t b = index % 8;
        bool result = (((bits_[i] << b) & 0x80) != 0);
        return result;
    }

    //! Sets the value of the bit at the given index (defaults to true)
    void set(size_t index, bool value = true)
    {
        size_t i = index / 8;
        if (i >= bits_.size())
            return;
        size_t b = index % 8;
        if (value)
            bits_[i] |= 0x80U >> b;
        else
            bits_[i] &= ~(0x80U >> b);
    }

    //! Sets the value of the bit at the given index to false.
    void reset(size_t index) { set(index, false); }

    //! Flips the value of the bit at the given index.
    void flip(size_t index) { set(index, !get(index)); }

    //! Returns the value of the bit at the given index
    bool operator [](size_t index) const { get(index); }

    //! Serializes the bits in the network format
    void serialize(std::vector<uint8_t> & out) const
    {
        P2p::VarArray<uint8_t>(bits_).serialize(out);
    }

private:

    std::vector<uint8_t> bits_;
};

} // namespace Network
