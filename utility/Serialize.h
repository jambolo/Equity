#pragma once

#include <vector>
#include <cstdint>

namespace Utility
{
    


template<typename T>
void serialize(T const & a, std::vector<uint8_t> & out)
{
    return a.serialize(out);
}

template<> void serialize<uint8_t>(uint8_t const & a, std::vector<uint8_t> & out);
template<> void serialize<uint16_t>(uint16_t const & a, std::vector<uint8_t> & out);
template<> void serialize<uint32_t>(uint32_t const & a, std::vector<uint8_t> & out);
template<> void serialize<uint64_t>(uint64_t const & a, std::vector<uint8_t> & out);
template<> void serialize<std::vector<uint8_t> >(std::vector<uint8_t> const & a, std::vector<uint8_t> & out);

template<typename T>
T deserialize(uint8_t const *& in, size_t & size)
{
    return T(in, size);
}

template<> uint8_t  deserialize<uint8_t >(uint8_t const *& in, size_t & size);
template<> uint16_t deserialize<uint16_t>(uint8_t const *& in, size_t & size);
template<> uint32_t deserialize<uint32_t>(uint8_t const *& in, size_t & size);
template<> uint64_t deserialize<uint64_t>(uint8_t const *& in, size_t & size);
std::vector<uint8_t>  deserializeBuffer(size_t n, uint8_t const *& in, size_t & size);

    class VarInt
    {
    public:
        VarInt(uint64_t v) : value_(v) {}
        VarInt(uint8_t const *& in, size_t & size);
        void serialize(std::vector<uint8_t> & out) const;
        uint64_t value() const { return value_; }
    private:
        uint64_t value_;
    };
    
    template<typename T>
    class VarArray
    {
    public:
        VarArray(std::vector<T> const & v) : vector_(v) {}
        VarArray(uint8_t const *& in, size_t & size)
        {
            VarInt arraySize(in, size);
            if (in == nullptr)
            {
                return;
            }
            vector_.reserve(arraySize.value());
            for (uint64_t i = 0; i < arraySize.value(); ++i)
            {
                T element = deserialize<T>(in, size);
                if (in == nullptr)
                {
                    return;
                }
                vector_.push_back(element);
            }
        }
        void serialize(std::vector<uint8_t> & out) const
        {
            VarInt(vector_.size()).serialize(out);
            for (auto const & element : vector_)
            {
                element.serialize(out);
            }
        }
        std::vector<T> value() const { return vector_; }
    private:
        std::vector<T> vector_;
    };
    
    template<>
    class VarArray<uint8_t>
    {
    public:
        VarArray(std::vector<uint8_t> const & v) : vector_(v) {}
        VarArray(uint8_t const *& in, size_t & size)
        {
            size_t arraySize = (size_t)VarInt(in, size).value();
            if (in == nullptr)
            {
                return;
            }
            if (size >= arraySize)
            {
                vector_.assign(in, in + arraySize);
                in += arraySize;
                size -= arraySize;
            }
        }
        void serialize(std::vector<uint8_t> & out) const
        {
            VarInt(vector_.size()).serialize(out);
            out.insert(out.end(), vector_.begin(), vector_.end());
        }
        std::vector<uint8_t> value() const { return vector_; }
    private:
        std::vector<uint8_t> vector_;
    };
    
} // namespace Utility
